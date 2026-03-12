#include "main.h"
#include <string.h>

s32 CREDITCOUNT;
struct CREDIT_s Credit [323];
f32 credit_speed;
f32 credit_time;

// Size: 0x8
struct CREDIT_s
{
    char* txt; // Offset: 0x0, DWARF: 0x188E9D
    short colour; // Offset: 0x4, DWARF: 0x188EC6
    short size; // Offset: 0x6, DWARF: 0x188EEF
};

static const unsigned long D3DSIMPLERENDERSTATEENCODE[82] = {
    0x00040260, 0x00040264, 0x00040268, 0x0004026C, 0x00040270, 0x00040274,
    0x00040278, 0x0004027C, 0x00040288, 0x0004028C, 0x00040A60, 0x00040A64,
    0x00040A68, 0x00040A6C, 0x00040A70, 0x00040A74, 0x00040A78, 0x00040A7C,
    0x00040A80, 0x00040A84, 0x00040A88, 0x00040A8C, 0x00040A90, 0x00040A94,
    0x00040A98, 0x00040A9C, 0x00040AA0, 0x00040AA4, 0x00040AA8, 0x00040AAC,
    0x00040AB0, 0x00040AB4, 0x00040AB8, 0x00040ABC, 0x00040AC0, 0x00040AC4,
    0x00040AC8, 0x00040ACC, 0x00040AD0, 0x00040AD4, 0x00040AD8, 0x00040ADC,
    0x000417F8, 0x00041E20, 0x00041E24, 0x00041E40, 0x00041E44, 0x00041E48,
    0x00041E4C, 0x00041E50, 0x00041E54, 0x00041E58, 0x00041E5C, 0x00041E60,
    0x00041D90, 0x00041E74, 0x00041E78, 0x00040354, 0x0004033C, 0x00040304,
    0x00040300, 0x00040340, 0x00040344, 0x00040348, 0x0004035C, 0x00040310,
    0x0004037C, 0x00040358, 0x00040374, 0x00040378, 0x00040364, 0x00040368,
    0x0004036C, 0x00040360, 0x00040350, 0x0004034C, 0x000409F8, 0x00040384,
    0x00040388, 0x00040330, 0x00040334, 0x00040338
};

static const unsigned long D3DTEXTUREDIRECTENCODE[4] = {
    0x00081B00, 0x00081B40, 0x00081B80, 0x00081BC0
};

static const unsigned long D3DDIRTYFROMRENDERSTATE[35] = {
    0x00002000, 0x00002000, 0x00002000, 0x00002000, 0x00002000, 0x00002000,
    0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F, 0x00001200, 0x00003000,
    0x00001000, 0x00001000, 0x00001000, 0x00001000, 0x00001000, 0x00001000,
    0x00001000, 0x00001000, 0x00001000, 0x00001000, 0x00001000, 0x00001000,
    0x00000100, 0x00000100, 0x00000900, 0x00000100, 0x00000100, 0x00000100,
    0x00000100, 0x00000100, 0x00000000, 0x00000000, 0x00000000
};

static const unsigned long D3DDIRTYFROMTEXTURESTATE[22] = {
    0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F,
    0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F,
    0x0000480F, 0x00000800, 0x00000800, 0x00000800, 0x00000800, 0x00000800,
    0x00000800, 0x00000800, 0x00000800, 0x00000400
};

const char lbl_8010875C[] = "KOVEL/FULLER";
const char lbl_8010876C[] = "BIG SESH STUDIOS";
const char lbl_80108780[] = "BENDER-HELPER-IMPACT";
const char lbl_80108798[] = "AXIOM DESIGN";
const char lbl_801087A8[] = "ABSINTHE PICTURES";
const char lbl_801087BC[] = "ADDITIONAL SPECIAL THANKS";
const char lbl_801087D8[] = "CHARLES YAM";
const char lbl_801087E4[] = "MEREDITH WOLLMAN";
const char lbl_801087F8[] = "FIONA WILSON";
const char lbl_80108808[] = "BRENT WATTS";
const char lbl_80108814[] = "SANDRA SHAGAT";
const char lbl_80108824[] = "TAMMY SCHACHTER";
const char lbl_80108834[] = "MARCUS SANFORD";
const char lbl_80108844[] = "RICCI RUKAVINA";
const char lbl_80108854[] = "SUZAN RUDE";
const char lbl_80108860[] = "NEAL ROBISON";
const char lbl_80108870[] = "JASON RECORD";
const char lbl_80108880[] = "BARRY KEHOE";
const char lbl_8010888C[] = "SCOTT JOHNSON";
const char lbl_8010889C[] = "SUESYN LAM";
const char lbl_801088A8[] = "GARY LAKE";
const char lbl_801088B4[] = "STUART HAY";
const char lbl_801088C0[] = "FLAVIE GUFFLET";
const char lbl_801088D0[] = "JAMES GALLOWAY";
const char lbl_801088E0[] = "VIRGINIA FOUT";
const char lbl_801088F0[] = "JOHN FOSTER";
const char lbl_801088FC[] = "HUGH BOWEN";
const char lbl_80108908[] = "GRACE BACA";
const char lbl_80108914[] = "LAUREN AZELTINE";
const char lbl_80108924[] = "BOOKLET DESIGN";
const char lbl_80108934[] = "WILLIAM PHAM";
const char lbl_80108944[] = "VINCENT DELUPIO";
const char lbl_80108954[] = "TIMOTHY PHILLIPS";
const char lbl_80108968[] = "TRISTAN ANDERSON";
const char lbl_8010897C[] = "SEEHE OH";
const char lbl_80108988[] = "RODRIGO REYES";
const char lbl_80108998[] = "MIRKO SEKULIC";
const char lbl_801089A8[] = "LESTER BROAS";
const char lbl_801089B8[] = "JULIET NIMMO";
const char lbl_801089C8[] = "JERRY NEYLAND";
const char lbl_801089D8[] = "FAUSTO LORENZANO";
const char lbl_801089EC[] = "DON CARROLL";
const char lbl_801089F8[] = "DAVEN DELIDLE";
const char lbl_80108A08[] = "BRIAN MATHISON";
const char lbl_80108A18[] = "BRIAN HIRAI";
const char lbl_80108A24[] = "BENJAMIN HINES";
const char lbl_80108A34[] = "AARON PARKER";
const char lbl_80108A44[] = "ALFRED LO";
const char lbl_80108A50[] = "QA TESTERS";
const char lbl_80108A5C[] = "JOSHUA TAPLEY";
const char lbl_80108A6C[] = "QA ASSOCIATE LEAD";
const char lbl_80108A80[] = "MICHAEL CARADONNA";
const char lbl_80108A94[] = "QA PROJECT LEAD";
const char lbl_80108AA4[] = "ASSURANCE TEAM";
const char lbl_80108AB4[] = "VIVENDI/UNIVERSAL QUALITY";
const char lbl_80108AD0[] = "MIKE RICHARDSON";
const char lbl_80108AE0[] = "DIRECTOR";
const char lbl_80108AEC[] = "ANDREW KLINE";
const char lbl_80108AFC[] = "CURTIS SHENTON";
const char lbl_80108B0C[] = "TEST MANAGERS";
const char lbl_80108B1C[] = "STEVE ALVARADO";
const char lbl_80108B2C[] = "KEITH TSUBOUCHI";
const char lbl_80108B3C[] = "STEVE FERREIRA";
const char lbl_80108B4C[] = "RICHARD BURKHART";
const char lbl_80108B60[] = "MIKE GONZALEZ";
const char lbl_80108B70[] = "RAY SCHREKENGOST";
const char lbl_80108B84[] = "BRIAN LEUNG";
const char lbl_80108B90[] = "PAUL GARCIA";
const char lbl_80108B9C[] = "KIM JARDIN";
const char lbl_80108BA8[] = "CARBLE CHEUNG";
const char lbl_80108BB8[] = "TESTERS";
const char lbl_80108BC0[] = "GALEN LAWS";
const char lbl_80108BCC[] = "JOSE VELASCO";
const char lbl_80108BDC[] = "SECOND LEAD TESTERS";
const char lbl_80108BF0[] = "CHRIS HO";
const char lbl_80108BFC[] = "LEAD TESTER";
const char lbl_80108C08[] = "ABSOLUTE QUALITY";
const char lbl_80108C1C[] = "QA TESTING";
const char lbl_80108C28[] = "THOM ANG";
const char lbl_80108C34[] = "ADDITIONAL CONCEPT ARTWORK";
const char lbl_80108C50[] = "CORY BURTON";
const char lbl_80108C5C[] = "N-TROPHY";
const char lbl_80108C68[] = "N-GIN";
const char lbl_80108C70[] = "JESS HARNELL";
const char lbl_80108C80[] = "LO-LO - THE AIR ELEMENTAL";
const char lbl_80108C9C[] = "MARK HAMILL";
const char lbl_80108CA8[] = "PY-RO - THE FIRE ELEMENTAL";
const char lbl_80108CC4[] = "R. LEE ERMY";
const char lbl_80108CD0[] = "WA-WA - THE WATER ELEMENTAL";
const char lbl_80108CEC[] = "TOM WILSON";
const char lbl_80108CF8[] = "ROK-KO - THE EARTH ELEMENTAL";
const char lbl_80108D18[] = "KEVIN MICHAEL RICHARDS";
const char lbl_80108D30[] = "CRUNCH BANDICOOT";
const char lbl_80108D44[] = "DEBI DERRYBERRY";
const char lbl_80108D54[] = "COCO BANDICOOT";
const char lbl_80108D64[] = "MEL WINKLER";
const char lbl_80108D70[] = "AKU-AKU";
const char lbl_80108D78[] = "CLANCY BROWN";
const char lbl_80108D88[] = "UKA-UKA";
const char lbl_80108D90[] = "DR. NEO CORTEX";
const char lbl_80108DA0[] = "VOICE CAST";
const char lbl_80108DAC[] = "HARRY WOOLWAY";
const char lbl_80108DBC[] = "JOHN ROBINSON";
const char lbl_80108DCC[] = "TOM JAEGER";
const char lbl_80108DD8[] = "RON HORWITZ";
const char lbl_80108DE4[] = "UNIVERSAL SOUND STUDIOS";
const char lbl_80108DFC[] = "SOUND EFFECTS";
const char lbl_80108E0C[] = "RIK SCHAEFFER";
const char lbl_80108E1C[] = "WOMB MUSIC";
const char lbl_80108E28[] = "DIALOG EDITING";
const char lbl_80108E38[] = "MARGARET TANG";
const char lbl_80108E48[] = "VOICE DIRECTOR AND CASTING";
const char lbl_80108E64[] = "JIM WILSON";
const char lbl_80108E70[] = "PRESIDENT";
const char lbl_80108E7C[] = "TORRIE DORRELL";
const char lbl_80108E8C[] = "VP OF GLOBAL MARKETING";
const char lbl_80108EA4[] = "ALEX SKILLMAN";
const char lbl_80108EB4[] = "SR. MANAGER OF PUBLIC RELATIONS";
const char lbl_80108ED4[] = "MICHAEL SEQUEIRA";
const char lbl_80108EE8[] = "CREATIVE SERVICES SUPERVISOR";
const char lbl_80108F08[] = "CHANDRA HILL";
const char lbl_80108F18[] = "DIRECTOR OF PROMOTIONS";
const char lbl_80108F30[] = "JASON COVEY";
const char lbl_80108F3C[] = "MARKETING COORDINATOR";
const char lbl_80108F54[] = "MARCUS SAVINO";
const char lbl_80108F64[] = "ASSOCIATE PRODUCT MANAGER";
const char lbl_80108F80[] = "CRAIG HOWE";
const char lbl_80108F8C[] = "SR. PRODUCT MANAGER";
const char lbl_80108FA0[] = "NICK TORCHIA";
const char lbl_80108FB0[] = "SEAN MOUNTAIN";
const char lbl_80108FC0[] = "MELISSA MILLER";
const char lbl_80108FD0[] = "ADDITIONAL PRODUCTION SUPPORT";
const char lbl_80108FF0[] = "VIJAY LAKSHMAN";
const char lbl_80109000[] = "VP OF PRODUCTION";
const char lbl_80109014[] = "CARLOS SCHULTE";
const char lbl_80109024[] = "PRODUCTION COORDINATOR";
const char lbl_8010903C[] = "SEAN KRANKEL";
const char lbl_8010904C[] = "DONOVAN SOTO";
const char lbl_8010905C[] = "ASSOCIATE PRODUCERS";
const char lbl_80109070[] = "DANIEL SUAREZ";
const char lbl_80109080[] = "SENIOR PRODUCER";
const char lbl_80109090[] = "STUDIOS";
const char lbl_80109098[] = "UNIVERSAL INTERACTIVE";
const char lbl_801090B0[] = "PUBLISHED BY";
const char lbl_801090C0[] = "ARRANGED BY";
const char lbl_801090CC[] = "MUTATO MUSIKA";
const char lbl_801090DC[] = "WRITTEN BY";
const char lbl_801090E8[] = "BANDICOOT THEME";
const char lbl_801090F8[] = "ORIGINAL CRASH";
const char lbl_80109108[] = "MARTEN JOUSTRA";
const char lbl_80109118[] = "ANDY BLYTHE";
const char lbl_80109124[] = "SWALLOW STUDIOS";
const char lbl_80109134[] = "MUSIC SOUNDTRACK";
const char lbl_80109148[] = "GILLIAN BIRCH";
const char lbl_80109158[] = "RICHARD CHEESEBOROUGH";
const char lbl_80109170[] = "CHRISTOPHER BUSH";
const char lbl_80109184[] = "SPECIAL THANKS";
const char lbl_80109194[] = "EMMA HOSKINS";
const char lbl_801091A4[] = "OFFICE MANAGER";
const char lbl_801091B4[] = "CRUNCH CONCEPTUAL DESIGN";
const char lbl_801091D0[] = "PRELIMINARY ARTWORK";
const char lbl_801091E4[] = "BARRY THOMPSON";
const char lbl_801091F4[] = "ADDITIONAL ARTWORK";
const char lbl_80109208[] = "LEON WARREN";
const char lbl_80109214[] = "CHARLES MCNAIR";
const char lbl_80109224[] = "WILLIAM THOMPSON";
const char lbl_80109238[] = "CUTSCENE ARTWORK";
const char lbl_8010924C[] = "BEVERLEY BUSH";
const char lbl_8010925C[] = "NICOLA DALY";
const char lbl_80109268[] = "CHARACTER ARTISTS";
const char lbl_8010927C[] = "CRAIG WHITTLE";
const char lbl_8010928C[] = "CONCEPT ARTIST";
const char lbl_8010929C[] = "ANTHONY WHITELEY";
const char lbl_801092B0[] = "CHRIS DICKER";
const char lbl_801092C0[] = "ANIMATORS";
const char lbl_801092CC[] = "JEREMY PARDON";
const char lbl_801092DC[] = "LEAD ANIMATOR";
const char lbl_801092EC[] = "PAUL DOBSON";
const char lbl_801092F8[] = "LEE BURNS";
const char lbl_80109304[] = "RHODA DALY";
const char lbl_80109310[] = "DAVE BURTON";
const char lbl_8010931C[] = "NEIL ALLEN";
const char lbl_80109328[] = "LEVEL ARTISTS";
const char lbl_80109338[] = "HEAD ARTIST";
const char lbl_80109344[] = "CHRIS PAYNE";
const char lbl_80109350[] = "JON ARDEN";
const char lbl_8010935C[] = "ANDREW HOLDROYD";
const char lbl_8010936C[] = "XBOX PROGRAMMERS";
const char lbl_80109380[] = "RICHARD TAYLOR";
const char lbl_80109390[] = "ALISTAIR CROWE";
const char lbl_801093A0[] = "GLYN SCRAGG";
const char lbl_801093AC[] = "ENGINE PROGRAMMERS";
const char lbl_801093C0[] = "STEVE MONKS";
const char lbl_801093CC[] = "DAVE DOOTSON";
const char lbl_801093DC[] = "LEAD ENGINE PROGRAMMERS";
const char lbl_801093F4[] = "MICHAEL JACKSON";
const char lbl_80109404[] = "RALPH FERNEYHOUGH";
const char lbl_80109418[] = "CHRIS STANFORTH";
const char lbl_80109428[] = "GAME PROGRAMMERS";
const char lbl_8010943C[] = "JOHN HODSKINSON";
const char lbl_8010944C[] = "LEAD GAME PROGRAMMER";
const char lbl_80109464[] = "JAMES CUNLIFFE";
const char lbl_80109474[] = "GAME AND LEVEL DESIGN";
const char lbl_8010948C[] = "ARTHUR PARSONS";
const char lbl_8010949C[] = "INTERNAL PRODUCER";
const char lbl_801094B0[] = "JON BURTON";
const char lbl_801094BC[] = "DIRECTED BY";
const char lbl_801094C8[] = "TRAVELLER'S TALES";
const char lbl_801094DC[] = "DEVELOPED BY";
const char lbl_801094EC[] = "ANTHONY SALWAY";
const char lbl_801094FC[] = "QA";
const char lbl_80109500[] = "CHRIS JACKSON";
const char lbl_80109510[] = "LEAD QA";
const char lbl_80109518[] = "MICHAEL ROBINSON";
const char lbl_8010952C[] = "QA MANAGER";
const char lbl_80109538[] = "GUY COCKCROFT";
const char lbl_80109548[] = "AUDIO";
const char lbl_80109550[] = "CHRIS JORDAN";
const char lbl_80109560[] = "BOB SMITH";
const char lbl_8010956C[] = "RICHARD PARKIN";
const char lbl_8010957C[] = "ADDITIONAL PROGRAMMING";
const char lbl_80109594[] = "KEVIN MARKS";
const char lbl_801095A0[] = "LEAD TOOLS PROGRAMMING";
const char lbl_801095B8[] = "MARK HETHERINGTON";
const char lbl_801095CC[] = "LEAD GAME/CONVERSION PROGRAMMING";
const char lbl_801095F0[] = "SOFTWARE";
const char lbl_801095FC[] = "EUROCOM ENTERTAINMENT";
const char lbl_80109614[] = "CONVERTED BY";
//NGC MATCH
void InitCredits(void) {
  struct CREDIT_s* credit;
  float size;

    credit = Credit;
    size = 0.0f;
    for (CREDITCOUNT = 0; credit->size > 0; CREDITCOUNT++,credit++) {
      size += (((float)credit->size) / 1000.0f) + (((float)credit->size) / 1000.0f);
    }
  size += 2.0f;
  credit_time = size / credit_speed;
  return;
}

//NGC MATCH
void DrawCredits(void) {
  struct CREDIT_s* credit;
  float xscale;
  float y;
  float size;
  s32 i;
  

  credit = Credit;
  size = GameTimer.ftime * credit_speed + (-1.0f);
    for (i = 0; i < CREDITCOUNT; i++,credit++) {
      xscale = (credit->size / 100.0f);
      if (((credit->txt != NULL) && (size > -1.5f)) && (size < 1.7f)) {
        strcpy(tbuf,credit->txt);
        if (strcmp(tbuf,lbl_80108EB4) == 0) {
            y = 0.85f;
        } else {
            y = 0.9f;
        }
        if (Game.language == 0x63) {
          AddSpacesIntoText(tbuf,1);
        }
        Text3D(tbuf, 0.0f, size, 1.0f,  xscale * y, xscale, xscale * 2, 2, credit->colour);
      }
      size = (size - ((xscale * 0.1f) + (xscale * 0.1f)));
    }
  return;
}
