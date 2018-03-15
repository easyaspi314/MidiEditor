// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "RegisteredParameterEvent.h"
#include "../midi/MidiTrack.h"

RegisteredParameterEvent::RegisteredParameterEvent(ubyte channel, MidiTrack *track) : MidiEvent(channel, track)
{

}
