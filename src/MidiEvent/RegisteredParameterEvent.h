#ifndef REGISTEREDPARAMETEREVENT_H
#define REGISTEREDPARAMETEREVENT_H

#include <QObject>
#include "MidiEvent.h"

class MidiTrack;

class RegisteredParameterEvent : public MidiEvent
{
    public:
        RegisteredParameterEvent(ubyte channel, MidiTrack *track);
};

#endif // REGISTEREDPARAMETEREVENT_H
