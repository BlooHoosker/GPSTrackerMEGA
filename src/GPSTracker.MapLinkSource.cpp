#include <GPSTracker.h>
#include <EEPROM.h>

void GPSTracker::setMapLinkSrc(const uint8_t linkSel){   
    EEPROM.update(LINK_SRC_ADDR, linkSel);
    _mapLinkSrc = linkSel;
}

void GPSTracker::getMapLinkSrc(){
    _mapLinkSrc = EEPROM[LINK_SRC_ADDR];
}