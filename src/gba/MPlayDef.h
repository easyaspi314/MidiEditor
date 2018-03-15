#ifndef MPLAYDEF_H
#define MPLAYDEF_H
#include "../Utils.h"
#include <QHash>
#include <QDebug>
#include "qlogging.h"

const QHash<QString,ubyte> *getMPlayDefs() {
    QHash<QString,ubyte> *table = new QHash<QString,ubyte>();

    table->reserve(405); // the size of these definitions.

    table->insert("W00", 0x80);           // WAIT
    table->insert("W01", 0x81);           //
    table->insert("W02", 0x82);           //
    table->insert("W03", 0x83);           //
    table->insert("W04", 0x84);           //
    table->insert("W05", 0x85);           //
    table->insert("W06", 0x86);           //
    table->insert("W07", 0x87);           //
    table->insert("W08", 0x88);           //
    table->insert("W09", 0x89);           //
    table->insert("W10", 0x8A);           //
    table->insert("W11", 0x8B);           //
    table->insert("W12", 0x8C);           //
    table->insert("W13", 0x8D);           //
    table->insert("W14", 0x8E);           //
    table->insert("W15", 0x8F);           //
    table->insert("W16", 0x90);           //
    table->insert("W17", 0x91);           //
    table->insert("W18", 0x92);           //
    table->insert("W19", 0x93);           //
    table->insert("W20", 0x94);           //
    table->insert("W21", 0x95);           //
    table->insert("W22", 0x96);           //
    table->insert("W23", 0x97);           //
    table->insert("W24", 0x98);           //
    table->insert("W28", 0x99);           //
    table->insert("W30", 0x9A);           //
    table->insert("W32", 0x9B);           //
    table->insert("W36", 0x9C);           //
    table->insert("W40", 0x9D);           //
    table->insert("W42", 0x9E);           //
    table->insert("W44", 0x9F);           //
    table->insert("W48", 0xA0);           //
    table->insert("W52", 0xA1);           //
    table->insert("W54", 0xA2);           //
    table->insert("W56", 0xA3);           //
    table->insert("W60", 0xA4);           //
    table->insert("W64", 0xA5);           //
    table->insert("W66", 0xA6);           //
    table->insert("W68", 0xA7);           //
    table->insert("W72", 0xA8);           //
    table->insert("W76", 0xA9);           //
    table->insert("W78", 0xAA);           //
    table->insert("W80", 0xAB);           //
    table->insert("W84", 0xAC);           //
    table->insert("W88", 0xAD);           //
    table->insert("W90", 0xAE);           //
    table->insert("W92", 0xAF);           //
    table->insert("W96", 0xB0);           //

    table->insert("FINE", 0xB1);          // fine
    table->insert("GOTO", 0xB2);          // goto
    table->insert("PATT", 0xB3);          // pattern play
    table->insert("PEND", 0xB4);          // pattern end
    table->insert("REPT", 0xB5);          // repeat
    table->insert("MEMACC", 0xB9);        // memacc op adr dat ***lib
    table->insert("PRIO", 0xBA);          // priority
    table->insert("TEMPO", 0xBB);         // tempo (BPM/2)
    table->insert("KEYSH", 0xBC);         // key shift
    table->insert("VOICE", 0xBD);         // voice #
    table->insert("VOL", 0xBE);           // volume
    table->insert("PAN", 0xBF);           // panpot (c_v+??)
    table->insert("BEND", 0xC0);          // pitch bend (c_v+??)
    table->insert("BENDR", 0xC1);         // bend range
    table->insert("LFOS", 0xC2);          // LFO speed
    table->insert("LFODL", 0xC3);         // LFO delay
    table->insert("MOD", 0xC4);           // modulation depth
    table->insert("MODT", 0xC5);          // modulation type
    table->insert("TUNE", 0xC8);          // micro tuning (c_v+??)

    table->insert("XCMD", 0xCD);          // extend command  ***lib
      table->insert("xIECV", 0x08);       //   imi.echo vol  ***lib
      table->insert("xIECL", 0x09);       //   imi.echo len  ***lib

    table->insert("EOT", 0xCE);           // End of Tie
    table->insert("TIE", 0xCF);           //
    table->insert("N01", 0xD0);           // NOTE
    table->insert("N02", 0xD1);           //
    table->insert("N03", 0xD2);           //
    table->insert("N04", 0xD3);           //
    table->insert("N05", 0xD4);           //
    table->insert("N06", 0xD5);           //
    table->insert("N07", 0xD6);           //
    table->insert("N08", 0xD7);           //
    table->insert("N09", 0xD8);           //
    table->insert("N10", 0xD9);           //
    table->insert("N11", 0xDA);           //
    table->insert("N12", 0xDB);           //
    table->insert("N13", 0xDC);           //
    table->insert("N14", 0xDD);           //
    table->insert("N15", 0xDE);           //
    table->insert("N16", 0xDF);           //
    table->insert("N17", 0xE0);           //
    table->insert("N18", 0xE1);           //
    table->insert("N19", 0xE2);           //
    table->insert("N20", 0xE3);           //
    table->insert("N21", 0xE4);           //
    table->insert("N22", 0xE5);           //
    table->insert("N23", 0xE6);           //
    table->insert("N24", 0xE7);           //
    table->insert("N28", 0xE8);           //
    table->insert("N30", 0xE9);           //
    table->insert("N32", 0xEA);           //
    table->insert("N36", 0xEB);           //
    table->insert("N40", 0xEC);           //
    table->insert("N42", 0xED);           //
    table->insert("N44", 0xEE);           //
    table->insert("N48", 0xEF);           //
    table->insert("N52", 0xF0);           //
    table->insert("N54", 0xF1);           //
    table->insert("N56", 0xF2);           //
    table->insert("N60", 0xF3);           //
    table->insert("N64", 0xF4);           //
    table->insert("N66", 0xF5);           //
    table->insert("N68", 0xF6);           //
    table->insert("N72", 0xF7);           //
    table->insert("N76", 0xF8);           //
    table->insert("N78", 0xF9);           //
    table->insert("N80", 0xFA);           //
    table->insert("N84", 0xFB);           //
    table->insert("N88", 0xFC);           //
    table->insert("N90", 0xFD);           //
    table->insert("N92", 0xFE);           //
    table->insert("N96", 0xFF);           //

    // maximum value for volume
    table->insert("mxv", 0x7F);           //

    // center value of PAN, BEND, TUNE
    table->insert("c_v", 0x40);           // -64 ~ +63

    // note for N??, TIE, EOT
    table->insert("CnM2", 0x00);          // C -2
    table->insert("CsM2", 0x01);          // C#-2
    table->insert("DnM2", 0x02);          // D -2
    table->insert("DsM2", 0x03);          // D#-2
    table->insert("EnM2", 0x04);          // E -2
    table->insert("FnM2", 0x05);          // F -2
    table->insert("FsM2", 0x06);          // F#-2
    table->insert("GnM2", 0x07);          // G -2
    table->insert("GsM2", 0x08);          // G#-2
    table->insert("AnM2", 0x09);          // A -2
    table->insert("AsM2", 0x0A);          // A#-2
    table->insert("BnM2", 0x0B);          // B -2
    table->insert("CnM1", 0x0C);          // C -1
    table->insert("CsM1", 0x0D);          // ...
    table->insert("DnM1", 0x0E);          //
    table->insert("DsM1", 0x0F);          //
    table->insert("EnM1", 0x10);          //
    table->insert("FnM1", 0x11);          //
    table->insert("FsM1", 0x12);          //
    table->insert("GnM1", 0x13);          //
    table->insert("GsM1", 0x14);          //
    table->insert("AnM1", 0x15);          //
    table->insert("AsM1", 0x16);          //
    table->insert("BnM1", 0x17);          //
    table->insert("Cn0", 0x18);           //
    table->insert("Cs0", 0x19);           //
    table->insert("Dn0", 0x1A);           //
    table->insert("Ds0", 0x1B);           //
    table->insert("En0", 0x1C);           //
    table->insert("Fn0", 0x1D);           //
    table->insert("Fs0", 0x1E);           //
    table->insert("Gn0", 0x1F);           //
    table->insert("Gs0", 0x20);           //
    table->insert("An0", 0x21);           //
    table->insert("As0", 0x22);           //
    table->insert("Bn0", 0x23);           //
    table->insert("Cn1", 0x24);           //
    table->insert("Cs1", 0x25);           //
    table->insert("Dn1", 0x26);           //
    table->insert("Ds1", 0x27);           //
    table->insert("En1", 0x28);           //
    table->insert("Fn1", 0x29);           //
    table->insert("Fs1", 0x2A);           //
    table->insert("Gn1", 0x2B);           //
    table->insert("Gs1", 0x2C);           //
    table->insert("An1", 0x2D);           //
    table->insert("As1", 0x2E);           //
    table->insert("Bn1", 0x2F);           //
    table->insert("Cn2", 0x30);           //
    table->insert("Cs2", 0x31);           //
    table->insert("Dn2", 0x32);           //
    table->insert("Ds2", 0x33);           //
    table->insert("En2", 0x34);           //
    table->insert("Fn2", 0x35);           //
    table->insert("Fs2", 0x36);           //
    table->insert("Gn2", 0x37);           //
    table->insert("Gs2", 0x38);           //
    table->insert("An2", 0x39);           //
    table->insert("As2", 0x3A);           //
    table->insert("Bn2", 0x3B);           //
    table->insert("Cn3", 0x3C);           //
    table->insert("Cs3", 0x3D);           //
    table->insert("Dn3", 0x3E);           //
    table->insert("Ds3", 0x3F);           //
    table->insert("En3", 0x40);           //
    table->insert("Fn3", 0x41);           //
    table->insert("Fs3", 0x42);           //
    table->insert("Gn3", 0x43);           //
    table->insert("Gs3", 0x44);           //
    table->insert("An3", 0x45);           // 440Hz
    table->insert("As3", 0x46);           //
    table->insert("Bn3", 0x47);           //
    table->insert("Cn4", 0x48);           //
    table->insert("Cs4", 0x49);           //
    table->insert("Dn4", 0x4A);           //
    table->insert("Ds4", 0x4B);           //
    table->insert("En4", 0x4C);           //
    table->insert("Fn4", 0x4D);           //
    table->insert("Fs4", 0x4E);           //
    table->insert("Gn4", 0x4F);           //
    table->insert("Gs4", 0x50);           //
    table->insert("An4", 0x51);           //
    table->insert("As4", 0x52);           //
    table->insert("Bn4", 0x53);           //
    table->insert("Cn5", 0x54);           //
    table->insert("Cs5", 0x55);           //
    table->insert("Dn5", 0x56);           //
    table->insert("Ds5", 0x57);           //
    table->insert("En5", 0x58);           //
    table->insert("Fn5", 0x59);           //
    table->insert("Fs5", 0x5A);           //
    table->insert("Gn5", 0x5B);           //
    table->insert("Gs5", 0x5C);           //
    table->insert("An5", 0x5D);           //
    table->insert("As5", 0x5E);           //
    table->insert("Bn5", 0x5F);           //
    table->insert("Cn6", 0x60);           //
    table->insert("Cs6", 0x61);           //
    table->insert("Dn6", 0x62);           //
    table->insert("Ds6", 0x63);           //
    table->insert("En6", 0x64);           //
    table->insert("Fn6", 0x65);           //
    table->insert("Fs6", 0x66);           //
    table->insert("Gn6", 0x67);           //
    table->insert("Gs6", 0x68);           //
    table->insert("An6", 0x69);           //
    table->insert("As6", 0x6A);           //
    table->insert("Bn6", 0x6B);           //
    table->insert("Cn7", 0x6C);           //
    table->insert("Cs7", 0x6D);           //
    table->insert("Dn7", 0x6E);           //
    table->insert("Ds7", 0x6F);           //
    table->insert("En7", 0x70);           //
    table->insert("Fn7", 0x71);           //
    table->insert("Fs7", 0x72);           //
    table->insert("Gn7", 0x73);           //
    table->insert("Gs7", 0x74);           //
    table->insert("An7", 0x75);           //
    table->insert("As7", 0x76);           //
    table->insert("Bn7", 0x77);           //
    table->insert("Cn8", 0x78);           //
    table->insert("Cs8", 0x79);           //
    table->insert("Dn8", 0x7A);           //
    table->insert("Ds8", 0x7B);           //
    table->insert("En8", 0x7C);           //
    table->insert("Fn8", 0x7D);           //
    table->insert("Fs8", 0x7E);           //
    table->insert("Gn8", 0x7F);           //

    // velocity
    table->insert("v000", 0x00);          //
    table->insert("v001", 0x01);          //
    table->insert("v002", 0x02);          //
    table->insert("v003", 0x03);          //
    table->insert("v004", 0x04);          //
    table->insert("v005", 0x05);          //
    table->insert("v006", 0x06);          //
    table->insert("v007", 0x07);          //
    table->insert("v008", 0x08);          //
    table->insert("v009", 0x09);          //
    table->insert("v010", 0x0A);          //
    table->insert("v011", 0x0B);          //
    table->insert("v012", 0x0C);          //
    table->insert("v013", 0x0D);          //
    table->insert("v014", 0x0E);          //
    table->insert("v015", 0x0F);          //
    table->insert("v016", 0x10);          //
    table->insert("v017", 0x11);          //
    table->insert("v018", 0x12);          //
    table->insert("v019", 0x13);          //
    table->insert("v020", 0x14);          //
    table->insert("v021", 0x15);          //
    table->insert("v022", 0x16);          //
    table->insert("v023", 0x17);          //
    table->insert("v024", 0x18);          //
    table->insert("v025", 0x19);          //
    table->insert("v026", 0x1A);          //
    table->insert("v027", 0x1B);          //
    table->insert("v028", 0x1C);          //
    table->insert("v029", 0x1D);          //
    table->insert("v030", 0x1E);          //
    table->insert("v031", 0x1F);          //
    table->insert("v032", 0x20);          //
    table->insert("v033", 0x21);          //
    table->insert("v034", 0x22);          //
    table->insert("v035", 0x23);          //
    table->insert("v036", 0x24);          //
    table->insert("v037", 0x25);          //
    table->insert("v038", 0x26);          //
    table->insert("v039", 0x27);          //
    table->insert("v040", 0x28);          //
    table->insert("v041", 0x29);          //
    table->insert("v042", 0x2A);          //
    table->insert("v043", 0x2B);          //
    table->insert("v044", 0x2C);          //
    table->insert("v045", 0x2D);          //
    table->insert("v046", 0x2E);          //
    table->insert("v047", 0x2F);          //
    table->insert("v048", 0x30);          //
    table->insert("v049", 0x31);          //
    table->insert("v050", 0x32);          //
    table->insert("v051", 0x33);          //
    table->insert("v052", 0x34);          //
    table->insert("v053", 0x35);          //
    table->insert("v054", 0x36);          //
    table->insert("v055", 0x37);          //
    table->insert("v056", 0x38);          //
    table->insert("v057", 0x39);          //
    table->insert("v058", 0x3A);          //
    table->insert("v059", 0x3B);          //
    table->insert("v060", 0x3C);          //
    table->insert("v061", 0x3D);          //
    table->insert("v062", 0x3E);          //
    table->insert("v063", 0x3F);          //
    table->insert("v064", 0x40);          //
    table->insert("v065", 0x41);          //
    table->insert("v066", 0x42);          //
    table->insert("v067", 0x43);          //
    table->insert("v068", 0x44);          //
    table->insert("v069", 0x4F);          //
    table->insert("v070", 0x46);          //
    table->insert("v071", 0x47);          //
    table->insert("v072", 0x48);          //
    table->insert("v073", 0x49);          //
    table->insert("v074", 0x4A);          //
    table->insert("v075", 0x4B);          //
    table->insert("v076", 0x4C);          //
    table->insert("v077", 0x4D);          //
    table->insert("v078", 0x4E);          //
    table->insert("v079", 0x4F);          //
    table->insert("v080", 0x50);          //
    table->insert("v081", 0x51);          //
    table->insert("v082", 0x52);          //
    table->insert("v083", 0x53);          //
    table->insert("v084", 0x54);          //
    table->insert("v085", 0x55);          //
    table->insert("v086", 0x56);          //
    table->insert("v087", 0x57);          //
    table->insert("v088", 0x58);          //
    table->insert("v089", 0x59);          //
    table->insert("v090", 0x5A);          //
    table->insert("v091", 0x5B);          //
    table->insert("v092", 0x5C);          //
    table->insert("v093", 0x5D);          //
    table->insert("v094", 0x5E);          //
    table->insert("v095", 0x5F);          //
    table->insert("v096", 0x60);          //
    table->insert("v097", 0x61);          //
    table->insert("v098", 0x62);          //
    table->insert("v099", 0x63);          //
    table->insert("v100", 0x64);          //
    table->insert("v101", 0x65);          //
    table->insert("v102", 0x66);          //
    table->insert("v103", 0x67);          //
    table->insert("v104", 0x68);          //
    table->insert("v105", 0x69);          //
    table->insert("v106", 0x6A);          //
    table->insert("v107", 0x6B);          //
    table->insert("v108", 0x6C);          //
    table->insert("v109", 0x6D);          //
    table->insert("v110", 0x6E);          //
    table->insert("v111", 0x6F);          //
    table->insert("v112", 0x70);          //
    table->insert("v113", 0x71);          //
    table->insert("v114", 0x72);          //
    table->insert("v115", 0x73);          //
    table->insert("v116", 0x74);          //
    table->insert("v117", 0x75);          //
    table->insert("v118", 0x76);          //
    table->insert("v119", 0x77);          //
    table->insert("v120", 0x78);          //
    table->insert("v121", 0x79);          //
    table->insert("v122", 0x7A);          //
    table->insert("v123", 0x7B);          //
    table->insert("v124", 0x7C);          //
    table->insert("v125", 0x7D);          //
    table->insert("v126", 0x7E);          //
    table->insert("v127", 0x7F);          //

    // exact gate time parameter for N??
    table->insert("gtp1", 0x01);          //
    table->insert("gtp2", 0x02);          //
    table->insert("gtp3", 0x03);          //

    // parameter of MODT
    table->insert("mod_vib", 0x00);       // vibrate
    table->insert("mod_tre", 0x01);       // tremolo
    table->insert("mod_pan", 0x02);       // auto-panpot

    // parameter of MEMACC
    table->insert("mem_set", 0x00);       //
    table->insert("mem_add", 0x01);       //
    table->insert("mem_sub", 0x02);       //
    table->insert("mem_mem_set", 0x03);   //
    table->insert("mem_mem_add", 0x04);   //
    table->insert("mem_mem_sub", 0x05);   //
    table->insert("mem_beq", 0x06);       //
    table->insert("mem_bne", 0x07);       //
    table->insert("mem_bhi", 0x08);       //
    table->insert("mem_bhs", 0x09);       //
    table->insert("mem_bls", 0x0A);       //
    table->insert("mem_blo", 0x0B);       //
    table->insert("mem_mem_beq", 0x0C);   //
    table->insert("mem_mem_bne", 0x0D);   //
    table->insert("mem_mem_bhi", 0x0E);   //
    table->insert("mem_mem_bhs", 0x0F);   //
    table->insert("mem_mem_bls", 0x10);   //
    table->insert("mem_mem_blo", 0x11);   //

    table->insert("reverb_set", 0x80);    // SOUND_MODE_REVERB_SET

    table->insert("PAM", 0xBF /*PAN*/);   //
    return table;
}
#endif // MPLAYDEF_H

