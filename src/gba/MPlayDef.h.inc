# 1 "MPlayDef.h"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "MPlayDef.h"



class exprtk {
public:
    template<typename T>
    class symbol_table {
    public:
        void add_constant(const char *x, const int y) {
        }
    };
};

exprtk::symbol_table<int> getMPlayDefs() {
exprtk::symbol_table<int> table;





const int W00 = 0x80; table.add_constant("W00", W00);
const int W01 = W00+1; table.add_constant("W01", W01);
const int W02 = W00+2; table.add_constant("W02", W02);
const int W03 = W00+3; table.add_constant("W03", W03);
const int W04 = W00+4; table.add_constant("W04", W04);
const int W05 = W00+5; table.add_constant("W05", W05);
const int W06 = W00+6; table.add_constant("W06", W06);
const int W07 = W00+7; table.add_constant("W07", W07);
const int W08 = W00+8; table.add_constant("W08", W08);
const int W09 = W00+9; table.add_constant("W09", W09);
const int W10 = W00+10; table.add_constant("W10", W10);
const int W11 = W00+11; table.add_constant("W11", W11);
const int W12 = W00+12; table.add_constant("W12", W12);
const int W13 = W00+13; table.add_constant("W13", W13);
const int W14 = W00+14; table.add_constant("W14", W14);
const int W15 = W00+15; table.add_constant("W15", W15);
const int W16 = W00+16; table.add_constant("W16", W16);
const int W17 = W00+17; table.add_constant("W17", W17);
const int W18 = W00+18; table.add_constant("W18", W18);
const int W19 = W00+19; table.add_constant("W19", W19);
const int W20 = W00+20; table.add_constant("W20", W20);
const int W21 = W00+21; table.add_constant("W21", W21);
const int W22 = W00+22; table.add_constant("W22", W22);
const int W23 = W00+23; table.add_constant("W23", W23);
const int W24 = W00+24; table.add_constant("W24", W24);
const int W28 = W00+25; table.add_constant("W28", W28);
const int W30 = W00+26; table.add_constant("W30", W30);
const int W32 = W00+27; table.add_constant("W32", W32);
const int W36 = W00+28; table.add_constant("W36", W36);
const int W40 = W00+29; table.add_constant("W40", W40);
const int W42 = W00+30; table.add_constant("W42", W42);
const int W44 = W00+31; table.add_constant("W44", W44);
const int W48 = W00+32; table.add_constant("W48", W48);
const int W52 = W00+33; table.add_constant("W52", W52);
const int W54 = W00+34; table.add_constant("W54", W54);
const int W56 = W00+35; table.add_constant("W56", W56);
const int W60 = W00+36; table.add_constant("W60", W60);
const int W64 = W00+37; table.add_constant("W64", W64);
const int W66 = W00+38; table.add_constant("W66", W66);
const int W68 = W00+39; table.add_constant("W68", W68);
const int W72 = W00+40; table.add_constant("W72", W72);
const int W76 = W00+41; table.add_constant("W76", W76);
const int W78 = W00+42; table.add_constant("W78", W78);
const int W80 = W00+43; table.add_constant("W80", W80);
const int W84 = W00+44; table.add_constant("W84", W84);
const int W88 = W00+45; table.add_constant("W88", W88);
const int W90 = W00+46; table.add_constant("W90", W90);
const int W92 = W00+47; table.add_constant("W92", W92);
const int W96 = W00+48; table.add_constant("W96", W96);

const int FINE = 0xb1; table.add_constant("FINE", FINE);
const int GOTO = 0xb2; table.add_constant("GOTO", GOTO);
const int PATT = 0xb3; table.add_constant("PATT", PATT);
const int PEND = 0xb4; table.add_constant("PEND", PEND);
const int REPT = 0xb5; table.add_constant("REPT", REPT);
const int MEMACC = 0xb9; table.add_constant("MEMACC", MEMACC);
const int PRIO = 0xba; table.add_constant("PRIO", PRIO);
const int TEMPO = 0xbb; table.add_constant("TEMPO", TEMPO);
const int KEYSH = 0xbc; table.add_constant("KEYSH", KEYSH);
const int VOICE = 0xbd; table.add_constant("VOICE", VOICE);
const int VOL = 0xbe; table.add_constant("VOL", VOL);
const int PAN = 0xbf; table.add_constant("PAN", PAN);
const int BEND = 0xc0; table.add_constant("BEND", BEND);
const int BENDR = 0xc1; table.add_constant("BENDR", BENDR);
const int LFOS = 0xc2; table.add_constant("LFOS", LFOS);
const int LFODL = 0xc3; table.add_constant("LFODL", LFODL);
const int MOD = 0xc4; table.add_constant("MOD", MOD);
const int MODT = 0xc5; table.add_constant("MODT", MODT);
const int TUNE = 0xc8; table.add_constant("TUNE", TUNE);

const int XCMD = 0xcd; table.add_constant("XCMD", XCMD);
 const int xIECV = 0x08; table.add_constant("xIECV", xIECV);
 const int xIECL = 0x09; table.add_constant("xIECL", xIECL);

const int EOT = 0xce; table.add_constant("EOT", EOT);
const int TIE = 0xcf; table.add_constant("TIE", TIE);
const int N01 = TIE+1; table.add_constant("N01", N01);
const int N02 = N01+1; table.add_constant("N02", N02);
const int N03 = N01+2; table.add_constant("N03", N03);
const int N04 = N01+3; table.add_constant("N04", N04);
const int N05 = N01+4; table.add_constant("N05", N05);
const int N06 = N01+5; table.add_constant("N06", N06);
const int N07 = N01+6; table.add_constant("N07", N07);
const int N08 = N01+7; table.add_constant("N08", N08);
const int N09 = N01+8; table.add_constant("N09", N09);
const int N10 = N01+9; table.add_constant("N10", N10);
const int N11 = N01+10; table.add_constant("N11", N11);
const int N12 = N01+11; table.add_constant("N12", N12);
const int N13 = N01+12; table.add_constant("N13", N13);
const int N14 = N01+13; table.add_constant("N14", N14);
const int N15 = N01+14; table.add_constant("N15", N15);
const int N16 = N01+15; table.add_constant("N16", N16);
const int N17 = N01+16; table.add_constant("N17", N17);
const int N18 = N01+17; table.add_constant("N18", N18);
const int N19 = N01+18; table.add_constant("N19", N19);
const int N20 = N01+19; table.add_constant("N20", N20);
const int N21 = N01+20; table.add_constant("N21", N21);
const int N22 = N01+21; table.add_constant("N22", N22);
const int N23 = N01+22; table.add_constant("N23", N23);
const int N24 = N01+23; table.add_constant("N24", N24);
const int N28 = N01+24; table.add_constant("N28", N28);
const int N30 = N01+25; table.add_constant("N30", N30);
const int N32 = N01+26; table.add_constant("N32", N32);
const int N36 = N01+27; table.add_constant("N36", N36);
const int N40 = N01+28; table.add_constant("N40", N40);
const int N42 = N01+29; table.add_constant("N42", N42);
const int N44 = N01+30; table.add_constant("N44", N44);
const int N48 = N01+31; table.add_constant("N48", N48);
const int N52 = N01+32; table.add_constant("N52", N52);
const int N54 = N01+33; table.add_constant("N54", N54);
const int N56 = N01+34; table.add_constant("N56", N56);
const int N60 = N01+35; table.add_constant("N60", N60);
const int N64 = N01+36; table.add_constant("N64", N64);
const int N66 = N01+37; table.add_constant("N66", N66);
const int N68 = N01+38; table.add_constant("N68", N68);
const int N72 = N01+39; table.add_constant("N72", N72);
const int N76 = N01+40; table.add_constant("N76", N76);
const int N78 = N01+41; table.add_constant("N78", N78);
const int N80 = N01+42; table.add_constant("N80", N80);
const int N84 = N01+43; table.add_constant("N84", N84);
const int N88 = N01+44; table.add_constant("N88", N88);
const int N90 = N01+45; table.add_constant("N90", N90);
const int N92 = N01+46; table.add_constant("N92", N92);
const int N96 = N01+47; table.add_constant("N96", N96);



const int mxv = 0x7F; table.add_constant("mxv", mxv);



const int c_v = 0x40; table.add_constant("c_v", c_v);



const int CnM2 = 0; table.add_constant("CnM2", CnM2);
const int CsM2 = 1; table.add_constant("CsM2", CsM2);
const int DnM2 = 2; table.add_constant("DnM2", DnM2);
const int DsM2 = 3; table.add_constant("DsM2", DsM2);
const int EnM2 = 4; table.add_constant("EnM2", EnM2);
const int FnM2 = 5; table.add_constant("FnM2", FnM2);
const int FsM2 = 6; table.add_constant("FsM2", FsM2);
const int GnM2 = 7; table.add_constant("GnM2", GnM2);
const int GsM2 = 8; table.add_constant("GsM2", GsM2);
const int AnM2 = 9; table.add_constant("AnM2", AnM2);
const int AsM2 = 10; table.add_constant("AsM2", AsM2);
const int BnM2 = 11; table.add_constant("BnM2", BnM2);
const int CnM1 = 12; table.add_constant("CnM1", CnM1);
const int CsM1 = 13; table.add_constant("CsM1", CsM1);
const int DnM1 = 14; table.add_constant("DnM1", DnM1);
const int DsM1 = 15; table.add_constant("DsM1", DsM1);
const int EnM1 = 16; table.add_constant("EnM1", EnM1);
const int FnM1 = 17; table.add_constant("FnM1", FnM1);
const int FsM1 = 18; table.add_constant("FsM1", FsM1);
const int GnM1 = 19; table.add_constant("GnM1", GnM1);
const int GsM1 = 20; table.add_constant("GsM1", GsM1);
const int AnM1 = 21; table.add_constant("AnM1", AnM1);
const int AsM1 = 22; table.add_constant("AsM1", AsM1);
const int BnM1 = 23; table.add_constant("BnM1", BnM1);
const int Cn0 = 24; table.add_constant("Cn0", Cn0);
const int Cs0 = 25; table.add_constant("Cs0", Cs0);
const int Dn0 = 26; table.add_constant("Dn0", Dn0);
const int Ds0 = 27; table.add_constant("Ds0", Ds0);
const int En0 = 28; table.add_constant("En0", En0);
const int Fn0 = 29; table.add_constant("Fn0", Fn0);
const int Fs0 = 30; table.add_constant("Fs0", Fs0);
const int Gn0 = 31; table.add_constant("Gn0", Gn0);
const int Gs0 = 32; table.add_constant("Gs0", Gs0);
const int An0 = 33; table.add_constant("An0", An0);
const int As0 = 34; table.add_constant("As0", As0);
const int Bn0 = 35; table.add_constant("Bn0", Bn0);
const int Cn1 = 36; table.add_constant("Cn1", Cn1);
const int Cs1 = 37; table.add_constant("Cs1", Cs1);
const int Dn1 = 38; table.add_constant("Dn1", Dn1);
const int Ds1 = 39; table.add_constant("Ds1", Ds1);
const int En1 = 40; table.add_constant("En1", En1);
const int Fn1 = 41; table.add_constant("Fn1", Fn1);
const int Fs1 = 42; table.add_constant("Fs1", Fs1);
const int Gn1 = 43; table.add_constant("Gn1", Gn1);
const int Gs1 = 44; table.add_constant("Gs1", Gs1);
const int An1 = 45; table.add_constant("An1", An1);
const int As1 = 46; table.add_constant("As1", As1);
const int Bn1 = 47; table.add_constant("Bn1", Bn1);
const int Cn2 = 48; table.add_constant("Cn2", Cn2);
const int Cs2 = 49; table.add_constant("Cs2", Cs2);
const int Dn2 = 50; table.add_constant("Dn2", Dn2);
const int Ds2 = 51; table.add_constant("Ds2", Ds2);
const int En2 = 52; table.add_constant("En2", En2);
const int Fn2 = 53; table.add_constant("Fn2", Fn2);
const int Fs2 = 54; table.add_constant("Fs2", Fs2);
const int Gn2 = 55; table.add_constant("Gn2", Gn2);
const int Gs2 = 56; table.add_constant("Gs2", Gs2);
const int An2 = 57; table.add_constant("An2", An2);
const int As2 = 58; table.add_constant("As2", As2);
const int Bn2 = 59; table.add_constant("Bn2", Bn2);
const int Cn3 = 60; table.add_constant("Cn3", Cn3);
const int Cs3 = 61; table.add_constant("Cs3", Cs3);
const int Dn3 = 62; table.add_constant("Dn3", Dn3);
const int Ds3 = 63; table.add_constant("Ds3", Ds3);
const int En3 = 64; table.add_constant("En3", En3);
const int Fn3 = 65; table.add_constant("Fn3", Fn3);
const int Fs3 = 66; table.add_constant("Fs3", Fs3);
const int Gn3 = 67; table.add_constant("Gn3", Gn3);
const int Gs3 = 68; table.add_constant("Gs3", Gs3);
const int An3 = 69; table.add_constant("An3", An3);
const int As3 = 70; table.add_constant("As3", As3);
const int Bn3 = 71; table.add_constant("Bn3", Bn3);
const int Cn4 = 72; table.add_constant("Cn4", Cn4);
const int Cs4 = 73; table.add_constant("Cs4", Cs4);
const int Dn4 = 74; table.add_constant("Dn4", Dn4);
const int Ds4 = 75; table.add_constant("Ds4", Ds4);
const int En4 = 76; table.add_constant("En4", En4);
const int Fn4 = 77; table.add_constant("Fn4", Fn4);
const int Fs4 = 78; table.add_constant("Fs4", Fs4);
const int Gn4 = 79; table.add_constant("Gn4", Gn4);
const int Gs4 = 80; table.add_constant("Gs4", Gs4);
const int An4 = 81; table.add_constant("An4", An4);
const int As4 = 82; table.add_constant("As4", As4);
const int Bn4 = 83; table.add_constant("Bn4", Bn4);
const int Cn5 = 84; table.add_constant("Cn5", Cn5);
const int Cs5 = 85; table.add_constant("Cs5", Cs5);
const int Dn5 = 86; table.add_constant("Dn5", Dn5);
const int Ds5 = 87; table.add_constant("Ds5", Ds5);
const int En5 = 88; table.add_constant("En5", En5);
const int Fn5 = 89; table.add_constant("Fn5", Fn5);
const int Fs5 = 90; table.add_constant("Fs5", Fs5);
const int Gn5 = 91; table.add_constant("Gn5", Gn5);
const int Gs5 = 92; table.add_constant("Gs5", Gs5);
const int An5 = 93; table.add_constant("An5", An5);
const int As5 = 94; table.add_constant("As5", As5);
const int Bn5 = 95; table.add_constant("Bn5", Bn5);
const int Cn6 = 96; table.add_constant("Cn6", Cn6);
const int Cs6 = 97; table.add_constant("Cs6", Cs6);
const int Dn6 = 98; table.add_constant("Dn6", Dn6);
const int Ds6 = 99; table.add_constant("Ds6", Ds6);
const int En6 = 100; table.add_constant("En6", En6);
const int Fn6 = 101; table.add_constant("Fn6", Fn6);
const int Fs6 = 102; table.add_constant("Fs6", Fs6);
const int Gn6 = 103; table.add_constant("Gn6", Gn6);
const int Gs6 = 104; table.add_constant("Gs6", Gs6);
const int An6 = 105; table.add_constant("An6", An6);
const int As6 = 106; table.add_constant("As6", As6);
const int Bn6 = 107; table.add_constant("Bn6", Bn6);
const int Cn7 = 108; table.add_constant("Cn7", Cn7);
const int Cs7 = 109; table.add_constant("Cs7", Cs7);
const int Dn7 = 110; table.add_constant("Dn7", Dn7);
const int Ds7 = 111; table.add_constant("Ds7", Ds7);
const int En7 = 112; table.add_constant("En7", En7);
const int Fn7 = 113; table.add_constant("Fn7", Fn7);
const int Fs7 = 114; table.add_constant("Fs7", Fs7);
const int Gn7 = 115; table.add_constant("Gn7", Gn7);
const int Gs7 = 116; table.add_constant("Gs7", Gs7);
const int An7 = 117; table.add_constant("An7", An7);
const int As7 = 118; table.add_constant("As7", As7);
const int Bn7 = 119; table.add_constant("Bn7", Bn7);
const int Cn8 = 120; table.add_constant("Cn8", Cn8);
const int Cs8 = 121; table.add_constant("Cs8", Cs8);
const int Dn8 = 122; table.add_constant("Dn8", Dn8);
const int Ds8 = 123; table.add_constant("Ds8", Ds8);
const int En8 = 124; table.add_constant("En8", En8);
const int Fn8 = 125; table.add_constant("Fn8", Fn8);
const int Fs8 = 126; table.add_constant("Fs8", Fs8);
const int Gn8 = 127; table.add_constant("Gn8", Gn8);



const int v000 = 0; table.add_constant("v000", v000);
const int v001 = 1; table.add_constant("v001", v001);
const int v002 = 2; table.add_constant("v002", v002);
const int v003 = 3; table.add_constant("v003", v003);
const int v004 = 4; table.add_constant("v004", v004);
const int v005 = 5; table.add_constant("v005", v005);
const int v006 = 6; table.add_constant("v006", v006);
const int v007 = 7; table.add_constant("v007", v007);
const int v008 = 8; table.add_constant("v008", v008);
const int v009 = 9; table.add_constant("v009", v009);
const int v010 = 10; table.add_constant("v010", v010);
const int v011 = 11; table.add_constant("v011", v011);
const int v012 = 12; table.add_constant("v012", v012);
const int v013 = 13; table.add_constant("v013", v013);
const int v014 = 14; table.add_constant("v014", v014);
const int v015 = 15; table.add_constant("v015", v015);
const int v016 = 16; table.add_constant("v016", v016);
const int v017 = 17; table.add_constant("v017", v017);
const int v018 = 18; table.add_constant("v018", v018);
const int v019 = 19; table.add_constant("v019", v019);
const int v020 = 20; table.add_constant("v020", v020);
const int v021 = 21; table.add_constant("v021", v021);
const int v022 = 22; table.add_constant("v022", v022);
const int v023 = 23; table.add_constant("v023", v023);
const int v024 = 24; table.add_constant("v024", v024);
const int v025 = 25; table.add_constant("v025", v025);
const int v026 = 26; table.add_constant("v026", v026);
const int v027 = 27; table.add_constant("v027", v027);
const int v028 = 28; table.add_constant("v028", v028);
const int v029 = 29; table.add_constant("v029", v029);
const int v030 = 30; table.add_constant("v030", v030);
const int v031 = 31; table.add_constant("v031", v031);
const int v032 = 32; table.add_constant("v032", v032);
const int v033 = 33; table.add_constant("v033", v033);
const int v034 = 34; table.add_constant("v034", v034);
const int v035 = 35; table.add_constant("v035", v035);
const int v036 = 36; table.add_constant("v036", v036);
const int v037 = 37; table.add_constant("v037", v037);
const int v038 = 38; table.add_constant("v038", v038);
const int v039 = 39; table.add_constant("v039", v039);
const int v040 = 40; table.add_constant("v040", v040);
const int v041 = 41; table.add_constant("v041", v041);
const int v042 = 42; table.add_constant("v042", v042);
const int v043 = 43; table.add_constant("v043", v043);
const int v044 = 44; table.add_constant("v044", v044);
const int v045 = 45; table.add_constant("v045", v045);
const int v046 = 46; table.add_constant("v046", v046);
const int v047 = 47; table.add_constant("v047", v047);
const int v048 = 48; table.add_constant("v048", v048);
const int v049 = 49; table.add_constant("v049", v049);
const int v050 = 50; table.add_constant("v050", v050);
const int v051 = 51; table.add_constant("v051", v051);
const int v052 = 52; table.add_constant("v052", v052);
const int v053 = 53; table.add_constant("v053", v053);
const int v054 = 54; table.add_constant("v054", v054);
const int v055 = 55; table.add_constant("v055", v055);
const int v056 = 56; table.add_constant("v056", v056);
const int v057 = 57; table.add_constant("v057", v057);
const int v058 = 58; table.add_constant("v058", v058);
const int v059 = 59; table.add_constant("v059", v059);
const int v060 = 60; table.add_constant("v060", v060);
const int v061 = 61; table.add_constant("v061", v061);
const int v062 = 62; table.add_constant("v062", v062);
const int v063 = 63; table.add_constant("v063", v063);
const int v064 = 64; table.add_constant("v064", v064);
const int v065 = 65; table.add_constant("v065", v065);
const int v066 = 66; table.add_constant("v066", v066);
const int v067 = 67; table.add_constant("v067", v067);
const int v068 = 68; table.add_constant("v068", v068);
const int v069 = 79; table.add_constant("v069", v069);
const int v070 = 70; table.add_constant("v070", v070);
const int v071 = 71; table.add_constant("v071", v071);
const int v072 = 72; table.add_constant("v072", v072);
const int v073 = 73; table.add_constant("v073", v073);
const int v074 = 74; table.add_constant("v074", v074);
const int v075 = 75; table.add_constant("v075", v075);
const int v076 = 76; table.add_constant("v076", v076);
const int v077 = 77; table.add_constant("v077", v077);
const int v078 = 78; table.add_constant("v078", v078);
const int v079 = 79; table.add_constant("v079", v079);
const int v080 = 80; table.add_constant("v080", v080);
const int v081 = 81; table.add_constant("v081", v081);
const int v082 = 82; table.add_constant("v082", v082);
const int v083 = 83; table.add_constant("v083", v083);
const int v084 = 84; table.add_constant("v084", v084);
const int v085 = 85; table.add_constant("v085", v085);
const int v086 = 86; table.add_constant("v086", v086);
const int v087 = 87; table.add_constant("v087", v087);
const int v088 = 88; table.add_constant("v088", v088);
const int v089 = 89; table.add_constant("v089", v089);
const int v090 = 90; table.add_constant("v090", v090);
const int v091 = 91; table.add_constant("v091", v091);
const int v092 = 92; table.add_constant("v092", v092);
const int v093 = 93; table.add_constant("v093", v093);
const int v094 = 94; table.add_constant("v094", v094);
const int v095 = 95; table.add_constant("v095", v095);
const int v096 = 96; table.add_constant("v096", v096);
const int v097 = 97; table.add_constant("v097", v097);
const int v098 = 98; table.add_constant("v098", v098);
const int v099 = 99; table.add_constant("v099", v099);
const int v100 = 100; table.add_constant("v100", v100);
const int v101 = 101; table.add_constant("v101", v101);
const int v102 = 102; table.add_constant("v102", v102);
const int v103 = 103; table.add_constant("v103", v103);
const int v104 = 104; table.add_constant("v104", v104);
const int v105 = 105; table.add_constant("v105", v105);
const int v106 = 106; table.add_constant("v106", v106);
const int v107 = 107; table.add_constant("v107", v107);
const int v108 = 108; table.add_constant("v108", v108);
const int v109 = 109; table.add_constant("v109", v109);
const int v110 = 110; table.add_constant("v110", v110);
const int v111 = 111; table.add_constant("v111", v111);
const int v112 = 112; table.add_constant("v112", v112);
const int v113 = 113; table.add_constant("v113", v113);
const int v114 = 114; table.add_constant("v114", v114);
const int v115 = 115; table.add_constant("v115", v115);
const int v116 = 116; table.add_constant("v116", v116);
const int v117 = 117; table.add_constant("v117", v117);
const int v118 = 118; table.add_constant("v118", v118);
const int v119 = 119; table.add_constant("v119", v119);
const int v120 = 120; table.add_constant("v120", v120);
const int v121 = 121; table.add_constant("v121", v121);
const int v122 = 122; table.add_constant("v122", v122);
const int v123 = 123; table.add_constant("v123", v123);
const int v124 = 124; table.add_constant("v124", v124);
const int v125 = 125; table.add_constant("v125", v125);
const int v126 = 126; table.add_constant("v126", v126);
const int v127 = 127; table.add_constant("v127", v127);



const int gtp1 = 1; table.add_constant("gtp1", gtp1);
const int gtp2 = 2; table.add_constant("gtp2", gtp2);
const int gtp3 = 3; table.add_constant("gtp3", gtp3);



const int mod_vib = 0; table.add_constant("mod_vib", mod_vib);
const int mod_tre = 1; table.add_constant("mod_tre", mod_tre);
const int mod_pan = 2; table.add_constant("mod_pan", mod_pan);



const int mem_set = 0; table.add_constant("mem_set", mem_set);
const int mem_add = 1; table.add_constant("mem_add", mem_add);
const int mem_sub = 2; table.add_constant("mem_sub", mem_sub);
const int mem_mem_set = 3; table.add_constant("mem_mem_set", mem_mem_set);
const int mem_mem_add = 4; table.add_constant("mem_mem_add", mem_mem_add);
const int mem_mem_sub = 5; table.add_constant("mem_mem_sub", mem_mem_sub);
const int mem_beq = 6; table.add_constant("mem_beq", mem_beq);
const int mem_bne = 7; table.add_constant("mem_bne", mem_bne);
const int mem_bhi = 8; table.add_constant("mem_bhi", mem_bhi);
const int mem_bhs = 9; table.add_constant("mem_bhs", mem_bhs);
const int mem_bls = 10; table.add_constant("mem_bls", mem_bls);
const int mem_blo = 11; table.add_constant("mem_blo", mem_blo);
const int mem_mem_beq = 12; table.add_constant("mem_mem_beq", mem_mem_beq);
const int mem_mem_bne = 13; table.add_constant("mem_mem_bne", mem_mem_bne);
const int mem_mem_bhi = 14; table.add_constant("mem_mem_bhi", mem_mem_bhi);
const int mem_mem_bhs = 15; table.add_constant("mem_mem_bhs", mem_mem_bhs);
const int mem_mem_bls = 16; table.add_constant("mem_mem_bls", mem_mem_bls);
const int mem_mem_blo = 17; table.add_constant("mem_mem_blo", mem_mem_blo);

const int reverb_set = 0x80; table.add_constant("reverb_set", reverb_set);
const int PAM = PAN; table.add_constant("PAM", PAM);
        return table;
}
