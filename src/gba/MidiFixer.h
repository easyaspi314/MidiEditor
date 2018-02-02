#ifndef GBA
#ifndef MIDIFIXER_H
#define MIDIFIXER_H

class MidiFile;

class MidiFixer {
	public:
		static void addAgbCompatibleEvents(MidiFile *midiFile, int modType);
		static void addModulationScale(MidiFile *midiFile, double modScale);
		static void combineVolumeAndExpression(MidiFile *midiFile);
		static void addExponentialScale(MidiFile *midiFile);
		static void removeRedundantMidiEvents(MidiFile *midiFile);
		static void fixLoopCarryBack(MidiFile *midiFile);
		static int minMax(double minVal, double val, double maxVal);
		static int expVol(int volume);
		static bool channelIsEmpty(MidiFile *midiFile, int channel);
};
struct agbControllerState
{
	int Tempo = 0x0;
	int Voice = 0xFF;
	int Volume = 0x7F;
	int Pan = 0x40;
	int BendR = 0xFF;
	int Bend = 0xFFFF;
	int Mod = 0x00;
};

#endif // MIDIFIXER_H
#endif
