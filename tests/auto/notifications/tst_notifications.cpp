/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/qobject.h>
#include <QtCore/qthread.h>

#include <Enginio/enginioclient.h>
#include <Enginio/enginioreply.h>
#include <Enginio/enginioidentity.h>

#include <Enginio/private/enginiobackendconnection_p.h>

#include "../common/common.h"

#define CHECK_NO_ERROR(response) \
    QVERIFY(!response->isError()); \
    QCOMPARE(response->errorType(), EnginioReply::NoError);\
    QCOMPARE(response->networkError(), QNetworkReply::NoError);\
    QVERIFY(response->backendStatus() >= 200 && response->backendStatus() < 300);

namespace EnginioTests {
    // FIXME: As soon as this functionality is deployed to the production server
    // this test has to be changed to use EnginioTests::TESTAPP_STAGING_URL.
    const QString TESTAPP_STAGING_URL(QStringLiteral("https://staging.engin.io"));
}

class tst_Notifications: public QObject
{
    Q_OBJECT

    QString _backendName;
    EnginioTests::EnginioBackendManager _backendManager;
    QByteArray _backendId;
    QByteArray _backendSecret;

public slots:
    void error(EnginioReply *reply) {
        qDebug() << "\n\n### ERROR";
        qDebug() << reply->errorString();
        reply->dumpDebugInfo();
        qDebug() << "\n###\n";
    }

private slots:
    void initTestCase();
    void cleanupTestCase();
    void populateWithData();
    void update_objects();
    void remove_objects();

private:
    void createObjectSchema();
};

void tst_Notifications::createObjectSchema()
{
    QJsonObject customObject1;
    customObject1["name"] = EnginioTests::CUSTOM_OBJECT1;
    QJsonObject customObject2;
    customObject2["name"] = EnginioTests::CUSTOM_OBJECT2;

    QJsonObject intValue;
    intValue["name"] = QStringLiteral("intValue");
    intValue["type"] = QStringLiteral("number");
    intValue["indexed"] = false;
    QJsonObject stringValue;
    stringValue["name"] = QStringLiteral("stringValue");
    stringValue["type"] = QStringLiteral("string");
    stringValue["indexed"] = true;

    QJsonObject settings;
    settings["websocket"] = true;
    QJsonArray properties;
    properties.append(intValue);
    properties.append(stringValue);
    customObject1["properties"] = properties;
    customObject2["properties"] = properties;
    customObject1["settings"] = settings;
    customObject2["settings"] = settings;

    QVERIFY(_backendManager.createObjectType(_backendName, EnginioTests::TESTAPP_ENV, customObject1));
    QVERIFY(_backendManager.createObjectType(_backendName, EnginioTests::TESTAPP_ENV, customObject2));
}

void tst_Notifications::initTestCase()
{
    // FIXME: Enable as soon as this can be run on the production server.
    // if (EnginioTests::TESTAPP_STAGING_URL.isEmpty())
    //    QFAIL("Needed environment variable ENGINIO_API_URL is not set!");

    qDebug() << "Running test against" << EnginioTests::TESTAPP_STAGING_URL;

    _backendManager.setServiceUrl(EnginioTests::TESTAPP_STAGING_URL);

    _backendName = QStringLiteral("Notifications") + QString::number(QDateTime::currentMSecsSinceEpoch());
    QVERIFY(_backendManager.createBackend(_backendName));

    QJsonObject apiKeys = _backendManager.backendApiKeys(_backendName, EnginioTests::TESTAPP_ENV);

    _backendId = apiKeys["backendId"].toString().toUtf8();
    _backendSecret = apiKeys["backendSecret"].toString().toUtf8();

    QVERIFY(!_backendId.isEmpty());
    QVERIFY(!_backendSecret.isEmpty());

    createObjectSchema();
}

void tst_Notifications::cleanupTestCase()
{
    QVERIFY(_backendManager.removeBackend(_backendName));
}

void tst_Notifications::populateWithData()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(_backendId);
    client.setBackendSecret(_backendSecret);
    client.setServiceUrl(EnginioTests::TESTAPP_STAGING_URL);

    EnginioBackendConnection connection;
    QSignalSpy notificationSpy(&connection, SIGNAL(dataReceived(QJsonObject)));
    connection.setServiceUrl(EnginioTests::TESTAPP_STAGING_URL);

    QJsonObject filter;
    filter["event"] = QStringLiteral("create");
    connection.connectToBackend(_backendId, _backendSecret, filter);
    QTRY_VERIFY(connection.isConnected());

    qDebug("Populating backend with data");
    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));
    int iterations = 10;
    int createdObjectCount = 0;

    {
        QJsonObject obj;
        obj["objectType"] = QString::fromUtf8("objects.").append(EnginioTests::CUSTOM_OBJECT1);
        EnginioReply *reply = client.query(obj);
        QTRY_VERIFY(reply->isFinished());
        QVERIFY(!reply->isError());
        QVERIFY2(!reply->data().isEmpty(), "Empty data in EnginioReply.");
        QVERIFY2(!reply->data()["results"].isUndefined(), "Undefined results array in EnginioReply data.");
        bool create = reply->data()["results"].toArray().isEmpty();
        spy.clear();

        if (create) {
            for (int i = 0; i < iterations; ++i)
            {
                obj["intValue"] = i;
                obj["stringValue"] = QString::fromUtf8("Query object");
                client.create(obj);
            }

            QTRY_COMPARE_WITH_TIMEOUT(spy.count(), iterations, 10000);
            QCOMPARE(spyError.count(), 0);
            createdObjectCount += iterations;
        }

        spy.clear();
    }
    {
        QJsonObject obj;
        obj["objectType"] = QString::fromUtf8("objects.").append(EnginioTests::CUSTOM_OBJECT2);
        EnginioReply *reply = client.query(obj);
        QTRY_VERIFY(reply->isFinished());
        QVERIFY(!reply->isError());
        QVERIFY2(!reply->data().isEmpty(), "Empty data in EnginioReply.");
        QVERIFY2(!reply->data()["results"].isUndefined(), "Undefined results array in EnginioReply data.");
        bool create = reply->data()["results"].toArray().isEmpty();
        spy.clear();

        if (create) {
            for (int i = 0; i < iterations; ++i)
            {
                obj["intValue"] = i;
                obj["stringValue"] = QString::fromUtf8("object test");
                client.create(obj);
            }

            QTRY_COMPARE_WITH_TIMEOUT(spy.count(), iterations, 10000);
            QCOMPARE(spyError.count(), 0);
            createdObjectCount += iterations;
        }
    }

    QTRY_COMPARE_WITH_TIMEOUT(notificationSpy.count(), createdObjectCount, 10000);

    for (int i = 0; i < notificationSpy.count(); ++i)
    {
        QJsonObject message = notificationSpy[i][0].value<QJsonObject>();
        QVERIFY(!message.isEmpty());
        QVERIFY(message["messageType"].isString());
        QVERIFY(message["messageType"].toString() == QStringLiteral("data"));
        QVERIFY(message["event"].isString());
        QVERIFY(message["event"].toString() == filter["event"].toString());
    }

    notificationSpy.clear();
    EnginioBackendConnection::WebSocketCloseStatus closeStatus = EnginioBackendConnection::GoingAwayCloseStatus;
    connection.close(closeStatus);
    QTRY_VERIFY_WITH_TIMEOUT(!connection.isConnected(), 10000);
    QCOMPARE(notificationSpy.count(), 1);

    QJsonObject message = notificationSpy[0][0].value<QJsonObject>();

    // We initiated the closing, so the server should send us
    // closeStatus in the closing message.
    QVERIFY(!message.isEmpty());
    QVERIFY(message["messageType"].isString());
    QVERIFY(message["messageType"].toString() == QStringLiteral("close"));
    QVERIFY(message["status"].isDouble());
    QVERIFY(message["status"].toDouble() == closeStatus);
}

void tst_Notifications::update_objects()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(_backendId);
    client.setBackendSecret(_backendSecret);
    client.setServiceUrl(EnginioTests::TESTAPP_STAGING_URL);

    EnginioBackendConnection connection;
    QSignalSpy notificationSpy(&connection, SIGNAL(dataReceived(QJsonObject)));
    connection.setServiceUrl(EnginioTests::TESTAPP_STAGING_URL);

    QJsonObject filter;
    filter["event"] = QStringLiteral("update");
    connection.connectToBackend(_backendId, _backendSecret, filter);
    QTRY_VERIFY(connection.isConnected());

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject query;
    query["objectType"] = QString::fromUtf8("objects.").append(EnginioTests::CUSTOM_OBJECT1);
    const EnginioReply* reply = client.query(query);
    QVERIFY(reply);

    QTRY_VERIFY(reply->isFinished());
    QVERIFY(!reply->isError());

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reply);
    CHECK_NO_ERROR(response);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QJsonArray array = data["results"].toArray();

    foreach (const QJsonValue value, array)
    {
        QVERIFY(value.isObject());
        QJsonObject obj = value.toObject();
        obj["stringValue"] = QString::fromUtf8("Update");
        client.update(obj);
    }

    QTRY_COMPARE(spy.count(), array.count() + 1);
    QCOMPARE(spyError.count(), 0);

    QTRY_COMPARE_WITH_TIMEOUT(notificationSpy.count(), array.count(), 10000);

    for (int i = 0; i < notificationSpy.count(); ++i)
    {
        QJsonObject message = notificationSpy[i][0].value<QJsonObject>();
        QVERIFY(!message.isEmpty());
        QVERIFY(message["messageType"].isString());
        QVERIFY(message["messageType"].toString() == QStringLiteral("data"));
        QVERIFY(message["event"].isString());
        QVERIFY(message["event"].toString() == filter["event"].toString());
    }

    notificationSpy.clear();
    EnginioBackendConnection::WebSocketCloseStatus closeStatus = EnginioBackendConnection::GoingAwayCloseStatus;
    connection.close(closeStatus);
    QTRY_VERIFY_WITH_TIMEOUT(!connection.isConnected(), 10000);
    QCOMPARE(notificationSpy.count(), 1);

    QJsonObject message = notificationSpy[0][0].value<QJsonObject>();

    // We initiated the closing, so the server should send us
    // closeStatus in the closing message.
    QVERIFY(!message.isEmpty());
    QVERIFY(message["messageType"].isString());
    QVERIFY(message["messageType"].toString() == QStringLiteral("close"));
    QVERIFY(message["status"].isDouble());
    QVERIFY(message["status"].toDouble() == closeStatus);
}

void tst_Notifications::remove_objects()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(_backendId);
    client.setBackendSecret(_backendSecret);
    client.setServiceUrl(EnginioTests::TESTAPP_STAGING_URL);

    EnginioBackendConnection connection;
    QSignalSpy notificationSpy(&connection, SIGNAL(dataReceived(QJsonObject)));
    connection.setServiceUrl(EnginioTests::TESTAPP_STAGING_URL);

    QJsonObject filter;
    filter["event"] = QStringLiteral("delete");
    connection.connectToBackend(_backendId, _backendSecret, filter);
    QTRY_VERIFY(connection.isConnected());

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject query;
    query["objectType"] = QString::fromUtf8("objects.").append(EnginioTests::CUSTOM_OBJECT1);
    const EnginioReply* reply = client.query(query);
    QVERIFY(reply);

    QTRY_VERIFY(reply->isFinished());
    QVERIFY(!reply->isError());

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reply);
    CHECK_NO_ERROR(response);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QJsonArray array = data["results"].toArray();

    foreach (const QJsonValue value, array)
    {
        QVERIFY(value.isObject());
        QJsonObject obj = value.toObject();
        query["id"] = obj["id"];
        client.remove(query);
    }

    QTRY_COMPARE(spy.count(), array.count() + 1);
    QCOMPARE(spyError.count(), 0);

    QTRY_COMPARE_WITH_TIMEOUT(notificationSpy.count(), array.count(), 10000);

    for (int i = 0; i < notificationSpy.count(); ++i)
    {
        QJsonObject message = notificationSpy[i][0].value<QJsonObject>();
        QVERIFY(!message.isEmpty());
        QVERIFY(message["messageType"].isString());
        QVERIFY(message["messageType"].toString() == QStringLiteral("data"));
        QVERIFY(message["event"].isString());
        QVERIFY(message["event"].toString() == filter["event"].toString());
    }

    notificationSpy.clear();
    EnginioBackendConnection::WebSocketCloseStatus closeStatus = EnginioBackendConnection::GoingAwayCloseStatus;
    connection.close(closeStatus);
    QTRY_VERIFY_WITH_TIMEOUT(!connection.isConnected(), 10000);
    QCOMPARE(notificationSpy.count(), 1);

    QJsonObject message = notificationSpy[0][0].value<QJsonObject>();

    // We initiated the closing, so the server should send us
    // closeStatus in the closing message.
    QVERIFY(!message.isEmpty());
    QVERIFY(message["messageType"].isString());
    QVERIFY(message["messageType"].toString() == QStringLiteral("close"));
    QVERIFY(message["status"].isDouble());
    QVERIFY(message["status"].toDouble() == closeStatus);
}

QTEST_MAIN(tst_Notifications)
#include "tst_notifications.moc"
