/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "Fact.h"

#include <QtCore/QObject>
#include <QtPositioning/QGeoCoordinate>

class EditPositionDialogController : public QObject
{
    Q_OBJECT
    
public:
    EditPositionDialogController(void);
    
    Q_PROPERTY(QGeoCoordinate   coordinate  READ coordinate WRITE setCoordinate NOTIFY coordinateChanged)
    Q_PROPERTY(Fact*            latitude    READ latitude                       CONSTANT)
    Q_PROPERTY(Fact*            longitude   READ longitude                      CONSTANT)
    Q_PROPERTY(Fact*            zone        READ zone                           CONSTANT)
    Q_PROPERTY(Fact*            hemisphere  READ hemisphere                     CONSTANT)
    Q_PROPERTY(Fact*            easting     READ easting                        CONSTANT)
    Q_PROPERTY(Fact*            northing    READ northing                       CONSTANT)
    Q_PROPERTY(Fact*            mgrs        READ mgrs                           CONSTANT)

    QGeoCoordinate  coordinate(void) const { return _coordinate; }
    Fact* latitude  (void) { return &_latitudeFact; }
    Fact* longitude (void) { return &_longitudeFact; }
    Fact* zone      (void) { return &_zoneFact; }
    Fact* hemisphere(void) { return &_hemisphereFact; }
    Fact* easting   (void) { return &_eastingFact; }
    Fact* northing  (void) { return &_northingFact; }
    Fact* mgrs      (void) { return &_mgrsFact; }

    void setCoordinate(QGeoCoordinate coordinate);

    Q_INVOKABLE void initValues(void);
    Q_INVOKABLE void setFromGeo(void);
    Q_INVOKABLE void setFromUTM(void);
    Q_INVOKABLE void setFromMGRS(void);
    Q_INVOKABLE void setFromVehicle(void);

signals:
    void coordinateChanged(QGeoCoordinate coordinate);

private:
    static QMap<QString, FactMetaData*> _metaDataMap;

    QGeoCoordinate _coordinate;

    Fact _latitudeFact;
    Fact _longitudeFact;
    Fact _zoneFact;
    Fact _hemisphereFact;
    Fact _eastingFact;
    Fact _northingFact;
    Fact _mgrsFact;

    static constexpr const char*  _latitudeFactName =      "Latitude";
    static constexpr const char*  _longitudeFactName =     "Longitude";
    static constexpr const char*  _zoneFactName =          "Zone";
    static constexpr const char*  _hemisphereFactName =    "Hemisphere";
    static constexpr const char*  _eastingFactName =       "Easting";
    static constexpr const char*  _northingFactName =      "Northing";
    static constexpr const char*  _mgrsFactName =          "MGRS";
};
