/*
	--------------------------------------------------
	RA8875 LCD/TFT Graphic Controller Driver Library
	--------------------------------------------------
	Version:0.69b19 bug fixes and adds from M.Sanderscock
	Added alternative pins for SPI (only Teensy 3.x or LC)
	Corrected setRotation and added absolute display W&H to support rotation
	Sleep mode on/off sequence ok!
	--> Fixed color at 8 bit! <--
	Added setFontAdvance
	Added some test BTE stuff
	Small optimizations and library cleaning
	
	++++++++++++++++++++++++++++++++++++++++++++++++++
	Written by: Max MC Costa for s.u.m.o.t.o.y
	++++++++++++++++++++++++++++++++++++++++++++++++++
An attemp to create a full featured library support for RA8875 chip from RAiO
that is alternative to the buggy and incomplete Adafruit_RA8875 (but thanks Ada 
and PaintYourDragon, great work aniway).

-------------------------------------------------------------------------------------
					>>>>>>>>>>>> About Copyrights <<<<<<<<<<<<<<<
-------------------------------------------------------------------------------------
Even if similar to Adafruit_RA8875 in some parts this library was builded originated
(as the Adafruit one) from the application note and datasheet from RAiO, however
I spent quite a lot of time to improve and bring to life all features so if you clone,
copy or modify please leave all my original notes intact!

-------------------------------------------------------------------------------------
				>>>>>>>>>>>>>>>>>>>>> Wiring <<<<<<<<<<<<<<<<<<<<<<<<<
-------------------------------------------------------------------------------------
TFT side	Teensy/Uno
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
CLK:		pin 13 UNO
MOSI:		pin 11 UNO
MISO:		pin 12 UNO
CS:			pin 10 (selectable but note 1*!)
INT:		pin 2  (selectable 2*)
RESET:		pin 9  (selectable and optional 0*)
*(0) In some modules you can leave unconnected but check if it's needed!
*(1) On Teensy3.x not all pin are usable for CS! Read the printed paper
that come with your Teensy3.x for more informations!
*(2) Arduino cannot use any pin for interrupts! In my examples I don't use
interrupts but just reading pin state so you can use any pin :)
-------------------------------------------------------------------------------------
TFT side	Stellaris (LM4F120XL) module=0 (still not checked)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
CLK:		xxx
MOSI:		xxx
MISO:		xxx
CS:			xxx  (selectable)
INT:		xxx  (selectable)
RESET:		xxx  (selectable and optional)

*if you use an [optional] SD card here's connections...
-------------------------------------------------------------------------------------
TFT side	Teensy2/Teensy3/Uno
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
SD CLK:		pin 13 [shared with RA]
SD MOSI:	pin 11 [shared with RA]
SD MISO:	pin 12 [shared with RA]
SD CS:		pin 4  (selectable 3*)
SD CARD ID: pin xx (selectable and optional)
*(3) On Teensy3.x not all pin are usable for CS! 
can be used: 2,6,9,10,15,20,21,22,23
-------------------------------------------------------------------------------------
TFT side	Stellaris (LM4F120XL) module=0 (still not checked)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
SD CLK:		xxx shared
SD MOSI:	xxx shared
SD MISO:	xxx shared
SD CS:		xxx  (selectable)
SD CARD ID: pin xx (selectable and optional)
-------------------------------------------------------------------------------------
TFT side	Teensy3 alternative setup
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
There are special cases on Teensy 3.x since it can be used for audio shield that remap
some pin. Using SPI in that situation will be impossible but you can use the alternative
pins definition feature of Teensy 3!
SD CLK:		pin 13(normal),pin 14(alt)  [shared with RA]
SD MOSI:	pin 11(normal),pin 7(alt) 	[shared with RA]
SD MISO:	pin 12(normal),pin 8(alt) 	[shared with RA]
SD CS:		pin 2  (selectable 3*)
SD CARD ID: pin xx (selectable and optional)
*(3) On Teensy3.x not all pin are usable for CS's! 
can be used: 2,6,9,10,15,20,21,22,23

*/

#ifndef _RA8875MC_H_
#define _RA8875MC_H_

#if defined(ENERGIA) // LaunchPad, FraunchPad and StellarPad specific
	#include "Energia.h"

	#undef byte
	#define byte      uint8_t

	#if defined(__TM4C129XNCZAD__) || defined(__TM4C1294NCPDT__)//tiva???
		#define NEEDS_SET_MODULE
	#elif defined(__LM4F120H5QR__) || defined(__TM4C123GH6PM__)//stellaris first version
		#define NEEDS_SET_MODULE
	#elif defined(__MSP430MCU__)//MSP430???
		// don't know
	#elif defined(TMS320F28069)//C2000???
		// don't know
	#elif defined(__CC3200R1M1RGC__)//CC3200???
		// don't know
	#endif
	static uint8_t SPImodule;
	static uint8_t SPDR;
#else
	#include "Arduino.h"
#endif

#include "Print.h"
#ifdef __AVR__
  #include <math.h>
#endif

enum RA8875sizes { RA8875_320x240, RA8875_480x272, RA8875_640x480, RA8875_800x480, Adafruit_480x272, Adafruit_640x480, Adafruit_800x480 };
enum RA8875modes { GRAPHIC,TEXT };
enum RA8875tcursor { NOCURSOR,IBEAM,UNDER,BLOCK };
enum RA8875tsize { X16,X24,X32 };
enum RA8875fontSource { INT, EXT };
enum RA8875fontCoding { ISO_IEC_8859_1, ISO_IEC_8859_2, ISO_IEC_8859_3, ISO_IEC_8859_4 };
enum RA8875extRomType { GT21L16T1W, GT21H16T1W, GT23L16U2W, GT30H24T3Y, GT23L24T3Y, GT23L24M1Z, GT23L32S4W, GT30H32S4W, GT30L32S4W, ER3303_1, ER3304_1 };
enum RA8875extRomCoding { GB2312, GB12345, BIG5, UNICODE, ASCII, UNIJIS, JIS0208, LATIN };
enum RA8875extRomFamily { STANDARD, ARIAL, ROMAN, BOLD };
enum RA8875boolean { LAYER1, LAYER2, TRANSPARENT, LIGHTEN, OR, AND, FLOATING };//for LTPR0
enum RA8875writes { L1, L2, CGRAM, PATTERN, CURSOR };//TESTING
enum RA8875scrollMode{ SIMULTANEOUS, LAYER1ONLY, LAYER2ONLY, BUFFERED };
enum RA8875pattern{ P8X8, P16X16 };
enum RA8875btedatam{ CONT, RECT };

/* ----------------------------DO NOT TOUCH ANITHING FROM HERE ------------------------*/
#include "_utility/RA8875Registers.h"
#include "_utility/RA8875ColorPresets.h"
#include "_utility/RA8875UserSettings.h"
#if !defined(USE_EXTERNALTOUCH)
#include "_utility/RA8875Calibration.h"
#endif

// Touch screen cal structs
typedef struct Point_TS { int32_t x; int32_t y; } tsPoint_t;//fix for DUE
typedef struct Matrix_TS { int32_t An,Bn,Cn,Dn,En,Fn,Divider ; } tsMatrix_t;//fix for DUE
static const uint8_t _RA8875colorMask[6] = {11,5,0,13,8,3};//for color masking, first 3 byte for 65K

class RA8875 : public Print {
 public:
//------------- Instance -------------------------
	//#if defined(__MKL26Z64__)
	//	RA8875(const uint8_t CS,const uint8_t RST=255,uint8_t spiInterface=0);//only Teensy LC
	#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__)
		RA8875(const uint8_t CS,const uint8_t RST=255,uint8_t mosi_pin=11,uint8_t sclk_pin=13,uint8_t miso_pin=12);
	#else	
		RA8875(const uint8_t CS, const uint8_t RST=255);//all the others
	#endif
//------------- Setup -------------------------
	void 		begin(const enum RA8875sizes s,uint8_t colors=16);
	//(RA8875_320x240, RA8875_480x272, RA8875_640x480, RA8875_800x480, Adafruit_480x272, Adafruit_640x480, Adafruit_800x480) , (8/16 bit)
//------------- Hardware related -------------------------
	void 		backlight(boolean on);
	void    	displayOn(boolean on);//turn diplay on/off
	void    	sleep(boolean sleep);//put display in sleep or not
	void 		brightness(uint8_t val);//ok
	uint8_t 	readStatus(void);//used to verify when an operation has concluded
	void		clearMemory(boolean full);//clear the RA8875 internal buffer (fully or current layer)
//------------ Low Level functions -------------------------
	void    	writeCommand(uint8_t d);
	void  		writeData16(uint16_t data);
//--------------area -------------------------------------
	void		setActiveWindow(uint16_t XL,uint16_t XR ,uint16_t YT ,uint16_t YB);//The working area where to draw on
	uint16_t 	width(void) const;//the phisical display width
	uint16_t 	height(void) const;//the phisical display height
	void		setRotation(uint8_t rotation); //rotate text and graphics
	uint8_t		getRotation(); //return the current rotation 0-3
//--------------color -------------------------------------
	void		setForegroundColor(uint16_t color);//color of objects in 16bit
	void		setForegroundColor(uint8_t R,uint8_t G,uint8_t B);//color of objects in 8+8+8bit
	void		setBackgroundColor(uint16_t color);//color of objects background in 16bit
	void		setBackgroundColor(uint8_t R,uint8_t G,uint8_t B);//color of objects background in 8+8+8bit
	void 		setTrasparentColor(uint16_t color);//the current transparent color in 16bit
	void 		setTrasparentColor(uint8_t R,uint8_t G,uint8_t B);//the current transparent color in 8+8+8bit
	void 		setColorBpp(uint8_t colors);//set the display color space 8 or 16!
<<<<<<< HEAD
	uint8_t 	getColorBpp();//get the current display color space (return 8 or 16)
	void		setRotation(uint8_t rotation); //rotate text and graphics
	uint8_t		getRotation(); //return the current rotation 0-3
//--------------area & color -------------------------
	void		setActiveWindow(uint16_t XL,uint16_t XR ,uint16_t YT ,uint16_t YB);
	uint16_t 	width(void);
	uint16_t 	height(void);
	void		setForegroundColor(uint16_t color);//OK
	void		setForegroundColor(uint8_t R,uint8_t G,uint8_t B);
	void		setBackgroundColor(uint16_t color);//OK
	void		setBackgroundColor(uint8_t R,uint8_t G,uint8_t B);
	void 		setTrasparentColor(uint16_t color);//OK
	void 		setTrasparentColor(uint8_t R,uint8_t G,uint8_t B);
//--------------Text functions ------------------------- 
	//----------cursor stuff................
	//void		showCursor(boolean cur,enum RA8875tcursor c=BLINK);//show text cursor, select cursor typ (NORMAL,BLINK)
	void 		setCursorBlinkRate(uint8_t rate);//0...255 0:faster
	void 		showCursor(enum RA8875tcursor c,bool blink);
	void    	setCursor(uint16_t x, uint16_t y);
=======
	uint8_t 	getColorBpp(void);//get the current display color space (return 8 or 16)
	inline uint16_t Color565(uint8_t r,uint8_t g,uint8_t b) { return ((b & 0xF8) << 8) | ((g & 0xFC) << 3) | (r >> 3); }
//--------------Cursor Stuff----------------------------
	void 		setCursorBlinkRate(uint8_t rate);//set blink rate of the cursor 0...255 0:faster
	void 		showCursor(enum RA8875tcursor c,bool blink);//show cursor(NOCURSOR,IBEAM,UNDER,BLOCK), default blinking
	void    	setCursor(uint16_t x, uint16_t y);//set cursor position to write text
>>>>>>> upstream/master
	void 		getCursor(uint16_t *x, uint16_t *y);//update the library _cursorX,_cursorY internally
	//and get the current data, this is useful sometime because the chip track cursor internally only
//--------------Text functions ------------------------- 
	void 		uploadUserChar(const uint8_t symbol[],uint8_t address);//upload user defined char as array at the address 0..255
	void		showUserChar(uint8_t symbolAddrs,uint8_t wide=0);//show user uploaded char at the adrs 0...255
	void    	setTextColor(uint16_t fColor, uint16_t bColor);//set text color + text background color
	void 		setTextColor(uint16_t fColor);//set text color (backgroud will be transparent)
	void    	setFontScale(uint8_t scale);//global font scale (w+h)
	void    	setFontScale(uint8_t vscale,uint8_t hscale);//font scale separatred bu w and h
	void    	setFontSize(enum RA8875tsize ts,boolean halfSize=false);//X16,X24,X32
	void 		setFontSpacing(uint8_t spc);//0:disabled ... 63:pix max
	void 		setFontRotate(boolean rot);//true = 90 degrees
	void 		setFontInterline(uint8_t pix);//0...63 pix
	void 		setFontFullAlign(boolean align);//mmmm... doesn't do nothing! Have to investigate
	void 		setFontAdvance(bool on);
	uint8_t 	getFontWidth(boolean inColums=false);
	uint8_t 	getFontHeight(boolean inRows=false);
	//----------Font Selection and related..............................
	void		setExternalFontRom(enum RA8875extRomType ert, enum RA8875extRomCoding erc,enum RA8875extRomFamily erf=STANDARD);
	void 		setFont(enum RA8875fontSource s);//INT,EXT (if you have a chip installed)
	void 		setIntFontCoding(enum RA8875fontCoding f);
	void		setExtFontFamily(enum RA8875extRomFamily erf,boolean setReg=true);
//--------------Graphic Funcions -------------------------
	void    	setXY(int16_t x, int16_t y);//graphic set location
	void 		setX(uint16_t x);
	void 		setY(uint16_t y) ;
	void 		setGraphicCursor(uint8_t cur);//0...7 Select a custom graphic cursor (you should upload first)
	void 		showGraphicCursor(boolean cur);//show graphic cursor
	void 		setPattern(uint8_t num, enum RA8875pattern p=P8X8);
	//--------------- DRAW -------------------------
	void    	drawPixel(int16_t x, int16_t y, uint16_t color);
	void    	drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);//ok
	void    	drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);//ok
	void    	fillScreen(uint16_t color=RA8875_BLACK);//fill the entire screen with a color(default black)
	void		clearScreen(uint16_t color=RA8875_BLACK);//exact as fillScreen, used for compatibility
	void    	drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
	void    	drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	void    	fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	void    	drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
	void    	fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
	void    	drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
	void    	fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
	void    	drawEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color);
	void    	fillEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color);
	void    	drawCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color);
	void    	fillCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color);
	void 		drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);//ok
	void 		fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
//-------------- LAYERS -----------------------------------------
	void 		useLayers(boolean on);//mainly used to turn of layers!
	void		writeTo(enum RA8875writes d);//L1, L2, CGRAM, PATTERN, CURSOR
	void 		layerEffect(enum RA8875boolean efx);//LAYER1, LAYER2, TRANSPARENT, LIGHTEN, OR, AND, FLOATING
	void 		layerTransparency(uint8_t layer1,uint8_t layer2);
	uint8_t		getCurrentLayer(void); //return the current drawing layer. If layers are OFF, return 255
//--------------- SCROLL ----------------------------------------
	void        setScrollMode(enum RA8875scrollMode mode); // The experimentalist
	void 		setScrollWindow(int16_t XL,int16_t XR ,int16_t YT ,int16_t YB);
	void 		scroll(uint16_t x,uint16_t y);
//-------------- DMA -------------------------------
	void 		drawFlashImage(int16_t x,int16_t y,int16_t w,int16_t h,uint8_t picnum);
//-------------- BTE --------------------------------------------
	void 		BTE_size(uint16_t w, uint16_t h);
	void	 	BTE_source(uint16_t SX,uint16_t DX ,uint16_t SY ,uint16_t DY);
	void		BTE_ROP_code(unsigned char setx);//TESTING
	void 		BTE_enable(bool on);//TESTING
	void 		BTE_dataMode(enum RA8875btedatam m);
//--------------GPIO & PWM -------------------------
	void    	GPIOX(boolean on);
	void    	PWMout(uint8_t pw,uint8_t p);//1:backlight, 2:free
//--------------Touch Screen -------------------------
#if !defined(USE_EXTERNALTOUCH)
	void 		touchBegin(uint8_t intPin);//prepare Touch Screen driver
	void    	touchEnable(boolean enabled);//enable/disable Touch Polling (disable INT)
	boolean 	touchDetect(boolean autoclear=false);//true=touch detected
	//Note:		must followed by touchReadxxx or use autoclear=true for standalone
	void 		touchReadRaw(uint16_t *x, uint16_t *y);//returns 10bit ADC data (0...1024)
	void 		touchReadPixel(uint16_t *x, uint16_t *y);//return pixels (0...width, 0...height)
	boolean		touchCalibrated(void);//true if screen calibration it's present
#endif
<<<<<<< HEAD
//----------------------------------------------------
	//Blue is interpreted differently in 8-bit mode - needs a different shift
    inline uint16_t Color565(uint8_t r,uint8_t g,uint8_t b) { return (_color_bpp==16) ? ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3) : ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 6); }
	void    	writeCommand(uint8_t d);
	void  		writeData16(uint16_t data);
=======
>>>>>>> upstream/master
//--------------Text Write -------------------------
virtual size_t write(uint8_t b) {
	textWrite((const char *)&b, 1);
	return 1;
}

virtual size_t write(const uint8_t *buffer, size_t size) {
	textWrite((const char *)buffer, size);
	return size;
}
using Print::write;

 private:
	//------------- VARS ----------------------------
	
	/*
	#if defined(SPI_HAS_TRANSACTION) && defined(__MKL26Z64__)
		uint8_t _SPIint;
	#endif
	*/

	uint8_t 		 		_cs, _rst;
	#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__)
		uint8_t _miso, _mosi, _sclk;
	#endif
	// Touch Screen vars ---------------------
	#if !defined(USE_EXTERNALTOUCH)
	uint8_t					_touchPin;
	bool					_clearTInt;
	uint16_t 				_tsAdcMinX,_tsAdcMinY,_tsAdcMaxX,_tsAdcMaxY;
	bool					_touchEnabled;
	#endif
	#if defined(USE_RA8875_KEYMATRIX)
	bool					_keyMatrixEnabled;
	#endif
	//----------------------------------------
	bool					_unsupported;//if true, not supported board
	bool					_inited;//true when init has been ended
	uint8_t					_initIndex;
	bool					_sleep;
	uint16_t 		 		_width, _height;
	uint16_t 		 		WIDTH, HEIGHT;
	uint16_t				_cursorX, _cursorY;//try to internally track text cursor...
	uint8_t 		 		_textHScale, _textVScale;	 		
	uint8_t					_rotation;
	uint8_t					_fontSpacing;
	uint8_t					_fontInterline;
	bool					_textWrap;
	bool					_fontFullAlig;
	bool					_fontRotation;
	uint8_t					_rotation;
	bool					_extFontRom;
	bool					_autoAdvance;
	enum RA8875extRomFamily _fontFamily;
	enum RA8875extRomType 	_fontRomType;
	enum RA8875extRomCoding _fontRomCoding;
	enum RA8875tsize		_textSize;
	enum RA8875modes 		_currentMode;
	enum RA8875sizes 		_size;
	enum RA8875fontSource 	_fontSource;
	enum RA8875tcursor		_textCursorStyle;
	//layer vars -----------------------------
	uint8_t					_maxLayers;
	bool					_useMultiLayers;
	uint8_t					_currentLayer;
	bool 					_hasLayerLimits;//helper
	//scroll vars ----------------------------
	int16_t					_scrollXL,_scrollXR,_scrollYT,_scrollYB;
	//color space-----------------------------
	uint8_t					_color_bpp;//8=256, 16=64K colors
	uint8_t					_brightness;
	//		functions --------------------------
	void 	initialize();
	void 	setSysClock(uint8_t pll1,uint8_t pll2,uint8_t pixclk);
	void    textWrite(const char* buffer, uint16_t len=0);//thanks to Paul Stoffregen for the initial version of this one
	void 	PWMsetup(uint8_t pw,boolean on, uint8_t clock);
	// 		helpers-----------------------------
	void 	checkLimitsHelper(int16_t &x,int16_t &y);//RA8875 it's prone to freeze with values out of range
	void 	circleHelper(int16_t x0, int16_t y0, int16_t r, uint16_t color, bool filled);
	void 	rectHelper(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool filled);
	void 	triangleHelper(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, bool filled);
	void 	ellipseHelper(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color, bool filled);
	void 	curveHelper(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color, bool filled);
	void 	lineAddressing(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
	void 	curveAddressing(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
	void 	roundRectHelper(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color, bool filled);
	#if !defined(USE_EXTERNALTOUCH)
	void	readTouchADC(uint16_t *x, uint16_t *y);
	void 	clearTouchInt(void);
	boolean touched(void);
	#endif
	void 	DMA_blockModeSize(int16_t BWR,int16_t BHR,int16_t SPWR);
	void 	DMA_startAddress(unsigned long adrs);
	//---------------- moved to private functions (before where public) ------------
	void 	changeMode(enum RA8875modes m);//GRAPHIC,TEXT (now private)
	void 	scanDirection(boolean invertH,boolean invertV);//(now private)
	//---------------------------------------------------------
    // Low level access  commands ----------------------
	void    	writeReg(uint8_t reg, uint8_t val);
	uint8_t 	readReg(uint8_t reg);
	void    	writeData(uint8_t data);
	uint8_t 	readData(bool stat=false);
	

	boolean 	waitPoll(uint8_t r, uint8_t f);//from adafruit
	void 		waitBusy(uint8_t res=0x80);//0x80, 0x40(BTE busy), 0x01(DMA busy)

	void 		startSend();
	void 		endSend();


	#if defined(NEEDS_SET_MODULE)//for Energia
	void 		selectCS(uint8_t module);
	#endif
	// Register containers -----------------------------------------
	// this needed to  prevent readRegister from chip that it's slow.
	uint8_t		_MWCR0Reg; //keep track of the register 		  [0x40]
	uint8_t		_DPCRReg;  ////Display Configuration		  	  [0x20]
	uint8_t		_FNCR0Reg; //Font Control Register 0 		  	  [0x21]
	uint8_t		_FNCR1Reg; //Font Control Register1 			  [0x22]
	uint8_t		_FWTSETReg; //Font Write Type Setting Register 	  [0x2E]
	uint8_t		_SFRSETReg; //Serial Font ROM Setting 		  	  [0x2F]
	uint8_t		_TPCR0Reg; //Touch Panel Control Register 0	  	  [0x70]
	uint8_t		_INTC1Reg; //Interrupt Control Register1		  [0xF0]
	//initialization parameters---------------------------------------------------------------------
	const uint8_t initStrings[4][15] = {
	{0x07,0x03,0x03,0x27,0x00,0x05,0x04,0x03,0xEF,0x00,0x05,0x00,0x0E,0x00,0x02},//0 -> 320x240 (0A)
	{0x07,0x03,0x82,0x3B,0x00,0x01,0x00,0x05,0x0F,0x01,0x02,0x00,0x07,0x00,0x09},//1 -> 480x272 (10)
	{0x07,0x03,0x01,0x4F,0x05,0x0F,0x01,0x00,0xDF,0x01,0x0A,0x00,0x0E,0x00,0x01},//2 -> 640x480
	{0x07,0x03,0x81,0x63,0x00,0x03,0x03,0x0B,0xDF,0x01,0x1F,0x00,0x16,0x00,0x01} //3 -> 800x480
	};
	//postburner PLL parameters (60Mhz)--------------------------------------------------------------
	const uint8_t sysClockPar[4][2] = {
	{0x0B,0x02},//0 -> 320x240
	{0x0B,0x02},//1 -> 480x272
	{0x0B,0x02},//2 -> 640x480
	{0x0B,0x02} //3 -> 800x480
	};
};

#endif
/*
Register affected by color
REG[10h] System Configuration Register (SYSR) bit 3,2//OK
REG[41h] Memory Write Control Register1 (MWCR1) bit 0//OK
REG[60h] Background Color Register 0 (BGCR0) bit 4..0//OK
REG[61h] Background Color Register 1 (BGCR1) bit 5..0//OK
REG[62h] Background Color Register 2 (BGCR2) bit 4..0//OK
REG[63h] Foreground Color Register 0 (FGCR0) bit 4..0//OK
REG[64h] Foreground Color Register 1 (FGCR1) bit 5..0//OK
REG[65h] Foreground Color Register 2 (FGCR2) bit 4..0//OK
REG[67h] Background Color Register for Transparent 0 (BGTR0) bit 4..0//OK
REG[68h] Background Color Register for Transparent 1 (BGTR1) bit 4..0//OK
REG[69h] Background Color Register for Transparent 2 (BGTR2) bit 4..0//OK
REG[84h] Graphic Cursor Color 0 (GCC0) bit 7..0
REG[85h] Graphic Cursor Color 1 (GCC1) bit 7..0

*/