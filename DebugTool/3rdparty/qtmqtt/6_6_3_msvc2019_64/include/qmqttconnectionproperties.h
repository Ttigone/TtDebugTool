// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QMQTTCONNECTIONPROPERTIES_H
#define QMQTTCONNECTIONPROPERTIES_H

// #include <QtMqtt/qmqttglobal.h>
// #include <QtMqtt/qmqtttype.h>
#include "qmqttglobal.h"
#include "qmqtttype.h"

#include <QtCore/QObject>
#include <QtCore/QSharedDataPointer>

QT_BEGIN_NAMESPACE

class QMqttConnectionPropertiesData;
class QMqttLastWillPropertiesData;
class QMqttServerConnectionPropertiesData;

class Q_MQTT_EXPORT QMqttLastWillProperties
{
public:
    QMqttLastWillProperties();
    QMqttLastWillProperties(const QMqttLastWillProperties &);
    QMqttLastWillProperties &operator=(const QMqttLastWillProperties &);
    ~QMqttLastWillProperties();

    quint32 willDelayInterval() const;
    QMqtt::PayloadFormatIndicator payloadFormatIndicator() const;
    quint32 messageExpiryInterval() const;
    QString contentType() const;
    QString responseTopic() const;
    QByteArray correlationData() const;
    QMqttUserProperties userProperties() const;

    void setWillDelayInterval(quint32 delay);
    void setPayloadFormatIndicator(QMqtt::PayloadFormatIndicator p);
    void setMessageExpiryInterval(quint32 expiry);
    void setContentType(const QString &content);
    void setResponseTopic(const QString &response);
    void setCorrelationData(const QByteArray &correlation);
    void setUserProperties(const QMqttUserProperties &properties);

private:
    QSharedDataPointer<QMqttLastWillPropertiesData> data;
};

class Q_MQTT_EXPORT QMqttConnectionProperties
{
public:
    QMqttConnectionProperties();
    QMqttConnectionProperties(const QMqttConnectionProperties &);
    QMqttConnectionProperties &operator=(const QMqttConnectionProperties &);
    ~QMqttConnectionProperties();

    quint32 sessionExpiryInterval() const;
    quint16 maximumReceive() const;
    quint32 maximumPacketSize() const;
    quint16 maximumTopicAlias() const;
    bool requestResponseInformation() const;
    bool requestProblemInformation() const;
    QMqttUserProperties userProperties() const;
    QString authenticationMethod() const;
    QByteArray authenticationData() const;

    void setSessionExpiryInterval(quint32 expiry);
    void setMaximumReceive(quint16 maximumReceive);
    void setMaximumPacketSize(quint32 packetSize);
    void setMaximumTopicAlias(quint16 alias);
    void setRequestResponseInformation(bool response);
    void setRequestProblemInformation(bool problem);
    void setUserProperties(const QMqttUserProperties &properties);
    void setAuthenticationMethod(const QString &authMethod);
    void setAuthenticationData(const QByteArray &authData);

private:
    friend class QMqttConnection;
    QSharedDataPointer<QMqttConnectionPropertiesData> data;
};

class Q_MQTT_EXPORT QMqttServerConnectionProperties
        : public QMqttConnectionProperties
{
public:
    enum ServerPropertyDetail : quint32 {
        None                            = 0x00000000,
        SessionExpiryInterval           = 0x00000001,
        MaximumReceive                  = 0x00000002,
        MaximumQoS                      = 0x00000004,
        RetainAvailable                 = 0x00000010,
        MaximumPacketSize               = 0x00000020,
        AssignedClientId                = 0x00000040,
        MaximumTopicAlias               = 0x00000080,
        ReasonString                    = 0x00000100,
        UserProperty                    = 0x00000200,
        WildCardSupported               = 0x00000400,
        SubscriptionIdentifierSupport   = 0x00000800,
        SharedSubscriptionSupport       = 0x00001000,
        ServerKeepAlive                 = 0x00002000,
        ResponseInformation             = 0x00004000,
        ServerReference                 = 0x00008000,
        AuthenticationMethod            = 0x00010000,
        AuthenticationData              = 0x00020000
    };
    Q_DECLARE_FLAGS(ServerPropertyDetails, ServerPropertyDetail)

    QMqttServerConnectionProperties();
    QMqttServerConnectionProperties(const QMqttServerConnectionProperties &);
    QMqttServerConnectionProperties &operator=(const QMqttServerConnectionProperties &);
    ~QMqttServerConnectionProperties();

    ServerPropertyDetails availableProperties() const;

    bool isValid() const;

    quint8 maximumQoS() const;
    bool retainAvailable() const;
    bool clientIdAssigned() const;
    QString reason() const;
    QMqtt::ReasonCode reasonCode() const;
    bool wildcardSupported() const;
    bool subscriptionIdentifierSupported() const;
    bool sharedSubscriptionSupported() const;
    quint16 serverKeepAlive() const;
    QString responseInformation() const;
    QString serverReference() const;

private:
    friend class QMqttConnection;
    QSharedDataPointer<QMqttServerConnectionPropertiesData> serverData;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QMqttServerConnectionProperties::ServerPropertyDetails)

QT_END_NAMESPACE

#endif // QMQTTCONNECTIONPROPERTIES_H
