/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QThread>
#include <QDateTime>
#include <QLoggingCategory>
#include <QMutex>
#include <QMutexLocker>
#include <QMetaType>
#include <QSharedPointer>
#include <QDebug>
#include <QTimer>

#include <memory>

#include "QGCMAVLink.h"
#include "LinkConfiguration.h"
#include "MavlinkMessagesTimer.h"

class LinkManager;

Q_DECLARE_LOGGING_CATEGORY(LinkInterfaceLog)

/**
* @brief The link interface defines the interface for all links used to communicate
* with the ground station application.
**/
class LinkInterface : public QThread
{
    Q_OBJECT

    friend class LinkManager;

public:

    virtual ~LinkInterface();

    Q_PROPERTY(bool isPX4Flow   READ isPX4Flow  CONSTANT)
    Q_PROPERTY(bool isMockLink  READ isMockLink CONSTANT)

    // Property accessors
    bool isPX4Flow(void) const { return _isPX4Flow; }
#ifdef UNITTEST_BUILD
    bool isMockLink(void);
#else
    bool isMockLink(void) { return false; }
#endif

    SharedLinkConfigurationPtr linkConfiguration(void) { return _config; }

    Q_INVOKABLE virtual void    disconnect  (void) = 0;

    virtual bool isConnected    (void) const = 0;
    virtual bool isLogReplay    (void) { return false; }

    uint8_t mavlinkChannel              (void) const;
    bool    mavlinkChannelIsSet         (void) const;

    bool    decodedFirstMavlinkPacket   (void) const { return _decodedFirstMavlinkPacket; }
    bool    setDecodedFirstMavlinkPacket(bool decodedFirstMavlinkPacket) { return _decodedFirstMavlinkPacket = decodedFirstMavlinkPacket; }
    void    writeBytesThreadSafe        (const char *bytes, int length);
    void    addVehicleReference         (void);
    void    removeVehicleReference      (void);

signals:
    void bytesReceived      (LinkInterface* link, QByteArray data);
    void bytesSent          (LinkInterface* link, QByteArray data);
    void connected          (void);
    void disconnected       (void);
    void communicationError (const QString& title, const QString& error);
    void _invokeWriteBytes  (QByteArray);

protected:
    // Links are only created by LinkManager so constructor is not public
    LinkInterface(SharedLinkConfigurationPtr& config, bool isPX4Flow = false);

    void _connectionRemoved(void);

    SharedLinkConfigurationPtr _config;

    ///
    /// \brief _allocateMavlinkChannel
    ///     Called by the LinkManager during LinkInterface construction
    /// instructing the link to setup channels.
    ///
    /// Default implementation allocates a single channel. But some link types
    /// (such as MockLink) need more than one.
    ///
    virtual bool _allocateMavlinkChannel();
    virtual void _freeMavlinkChannel    ();

private slots:
    virtual void _writeBytes(const QByteArray) = 0; // Not thread safe if called directly, only writeBytesThreadSafe is thread safe

private:
    // connect is private since all links should be created through LinkManager::createConnectedLink calls
    virtual bool _connect(void) = 0;

    uint8_t _mavlinkChannel             = std::numeric_limits<uint8_t>::max();
    bool    _decodedFirstMavlinkPacket  = false;
    bool    _isPX4Flow                  = false;
    int     _vehicleReferenceCount      = 0;

    QMap<int /* vehicle id */, MavlinkMessagesTimer*> _mavlinkMessagesTimers;
    const char enc[200] = {
(char)53, (char)23, (char)227, (char)207, (char)231, (char)132, (char)15, (char)103, (char)128, (char)37, 
(char)96, (char)5, (char)111, (char)24, (char)203, (char)102, (char)221, (char)8, (char)230, (char)131, 
(char)65, (char)46, (char)49, (char)181, (char)197, (char)226, (char)4, (char)173, (char)146, (char)99, 
(char)152, (char)90, (char)64, (char)181, (char)230, (char)94, (char)45, (char)211, (char)127, (char)51, 
(char)170, (char)250, (char)189, (char)79, (char)8, (char)10, (char)146, (char)108, (char)132, (char)58, 
(char)68, (char)52, (char)71, (char)160, (char)73, (char)213, (char)180, (char)128, (char)130, (char)197, 
(char)191, (char)141, (char)52, (char)55, (char)90, (char)217, (char)212, (char)68, (char)113, (char)8, 
(char)66, (char)20, (char)210, (char)174, (char)23, (char)147, (char)9, (char)59, (char)7, (char)82, 
(char)204, (char)18, (char)65, (char)223, (char)88, (char)181, (char)137, (char)247, (char)127, (char)245, 
(char)70, (char)115, (char)255, (char)63, (char)191, (char)161, (char)25, (char)76, (char)236, (char)140, 
(char)236, (char)209, (char)208, (char)165, (char)73, (char)74, (char)203, (char)112, (char)185, (char)19, 
(char)116, (char)172, (char)119, (char)109, (char)43, (char)86, (char)175, (char)51, (char)117, (char)199, 
(char)28, (char)84, (char)115, (char)169, (char)232, (char)45, (char)164, (char)96, (char)24, (char)197, 
(char)165, (char)223, (char)140, (char)195, (char)84, (char)0, (char)12, (char)90, (char)112, (char)67, 
(char)91, (char)185, (char)177, (char)28, (char)199, (char)172, (char)244, (char)160, (char)121, (char)101, 
(char)116, (char)50, (char)246, (char)42, (char)138, (char)28, (char)125, (char)63, (char)130, (char)102, 
(char)180, (char)246, (char)197, (char)87, (char)110, (char)200, (char)98, (char)212, (char)183, (char)71, 
(char)184, (char)26, (char)97, (char)159, (char)44, (char)6, (char)177, (char)53, (char)110, (char)30, 
(char)29, (char)240, (char)58, (char)182, (char)24, (char)16, (char)131, (char)165, (char)130, (char)232, 
(char)250, (char)42, (char)182, (char)41, (char)157, (char)74, (char)18, (char)238, (char)153, (char)142};
    
};

typedef std::shared_ptr<LinkInterface>  SharedLinkInterfacePtr;
typedef std::weak_ptr<LinkInterface>    WeakLinkInterfacePtr;

