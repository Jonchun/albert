// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QApplication>
#include <QDateTime>
#include <QJsonDocument>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTimer>
#include "telemetry.h"
using namespace Core;
using namespace std;
using namespace std::chrono;

namespace {
const QString config_key = "telemetry";
}

Telemetry::Telemetry()
{
    QObject::connect(&timer_, &QTimer::timeout, [this]{trySendReport();});

    QSettings settings(qApp->applicationName());
    if ( !settings.contains(config_key) ) {
        QMessageBox mb(QMessageBox::Question, "Albert telemetry",
                       "Albert collects anonymous data to improve user "
                       "experience and plot nice charts (upcoming feature). "
                       "Further a high usercount helps me to stay motivated "
                       "working on albert. ;)\n\nYou can check the data "
                       "sent in the details.\n\nDo you want to send "
                       "telemetry data?",
                    QMessageBox::No|QMessageBox::Yes);
        mb.setDefaultButton(QMessageBox::Yes);
        mb.setDetailedText(QJsonDocument(buildReport()).toJson(QJsonDocument::Indented));
        settings.setValue(config_key, mb.exec() == QMessageBox::Yes);
    }
    enable(settings.value(config_key).toBool());
}

void Core::Telemetry::enable(bool enable)
{
    if (enable) {
        trySendReport();
        timer_.start(60000);
    }
    else {
        timer_.stop();
    }
    QSettings(qApp->applicationName()).setValue(config_key, enable);
}

void Core::Telemetry::trySendReport()
{

    QSqlQuery q(QSqlDatabase::database());

    // Get timestamp of last report
    if (!q.exec("SELECT value FROM conf WHERE key=\"last_report\"; "))
        qFatal("Unable to get last_report from conf: %s", q.lastError().text().toUtf8().constData());
    int64_t last_report = 0;
    if (q.next())
        last_report = q.value(0).toLongLong();

    // If not reported today
    if (QDateTime::fromMSecsSinceEpoch(last_report*1000).date() != QDate::currentDate()) {

        QJsonObject object = buildReport();

        QString addr = "Zffb,!!*\" $## $\"' **!";
        for ( auto &c: addr)
            c.unicode()=c.unicode()+14;

        static QNetworkAccessManager *manager = new QNetworkAccessManager;
        QNetworkRequest request((QUrl(addr)));
        request.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json"));
        QNetworkReply* reply = manager->put(request, QJsonDocument(object).toJson(QJsonDocument::Compact));
        QObject::connect(reply, &QNetworkReply::finished, [reply](){
            if (reply->error() == QNetworkReply::NoError){
                qDebug() << "Report sent.";
                // Store time of last report
                QSqlQuery q(QSqlDatabase::database());
                q.prepare("INSERT OR REPLACE INTO conf VALUES(\"last_report\", :ts); ");
                q.bindValue(":ts", static_cast<qint64>(QDateTime::currentMSecsSinceEpoch()/1000));
                if (!q.exec())
                    qFatal("Could not set last_report: %s %s", qPrintable(q.executedQuery()), qPrintable(q.lastError().text()));
            }
            reply->deleteLater();
        });
    }

}

QJsonObject Telemetry::buildReport()
{
    QSqlQuery q(QSqlDatabase::database());

    // Get timestamp of last report
    if (!q.exec("SELECT value FROM conf WHERE key=\"last_report\"; "))
        qFatal("Unable to get last_report from conf: %s", q.lastError().text().toUtf8().constData());
    int64_t last_report = 0;
    if (q.next())
        last_report = q.value(0).toLongLong();

    // Get activations
    q.prepare("SELECT count(*) "
              "FROM activation a "
              "JOIN query q "
              "ON a.query_id = q.id "
              "WHERE :since < q.timestamp;");
    q.bindValue(":since", static_cast<qint64>(last_report));
    if (!q.exec())
        qFatal("SQL ERROR: %s %s", qPrintable(q.executedQuery()), qPrintable(q.lastError().text()));
    if (!q.next())
        qFatal("Could not compute activations.");
    int64_t activations = q.value(0).toLongLong();


    QJsonObject object;
    object.insert("version", qApp->applicationVersion());
    object.insert("os", QSysInfo::prettyProductName());
    object.insert("os_version", QSysInfo::productVersion());
    object.insert("activations", static_cast<qint64>(activations));
    return object;
}