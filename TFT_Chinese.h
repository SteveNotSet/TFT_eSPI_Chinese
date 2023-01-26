#include "HZK16.h"
#include "utf8togb2312.hpp"
void putHanzi(TFT_eSPI tft, uint16_t x, uint16_t y, uint16_t GBcode, uint16_t fcolor, uint16_t bcolor){
	uint32_t offset = (94*(unsigned int)((GBcode>>8)-0xa0-1)+((GBcode&0x00FF)-0xa0-1))*32;
	uint8_t buf;
	uint8_t key[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
	tft.setAddrWindow(x,y,16,16);
	tft.startWrite();
	for(uint8_t i = 0; i<16; i++){
		for(uint8_t j = 0; j<2; j++){
			buf = pgm_read_byte(HZK16 + offset + 2*i + j);
			for(uint8_t k = 0; k <8; k ++){
				if(buf & key[k]){
					tft.writedata((uint8_t)(fcolor>>8));
					tft.writedata((uint8_t)fcolor);
//					Serial.print("#");
				}else{
					tft.writedata((uint8_t)(bcolor>>8));
					tft.writedata((uint8_t)bcolor);
//					Serial.print("_");
				}
			}
//			Serial.print("AA");
		}
//		Serial.println();
	}
//	Serial.println("%%%%");
	tft.endWrite();
}

void printGBK(TFT_eSPI& tft, uint16_t* chinese, uint32_t len, uint16_t textColor, uint16_t bgColor){
	//tft.setTextColor(textColor, bgColor);
	for(uint32_t i = 0; i < len; i++){
		if(tft.getCursorX()+16>tft.width()){
			tft.setCursor(0,tft.getCursorY()+16);
		}
		if(chinese[i]!=0){
			putHanzi(tft, tft.getCursorX(), tft.getCursorY(), chinese[i], textColor, bgColor);
			tft.setCursor(tft.getCursorX()+16, tft.getCursorY());
		}
	}
}

void printChineseString(TFT_eSPI& tft, String text, uint16_t textColor, uint16_t bgColor){
	uint32_t startPos, endPos;
	uint16_t* gbkBuf;
	uint32_t chineseLen, textLen;
	startPos = 0;
	endPos = 0;
	textLen = text.length();
	if(textLen == 0){
		return;
	}
	tft.setTextColor(textColor, bgColor);
	tft.setTextFont(2);
	//do{
		if((text.charAt(startPos)<0x80)&&(text.charAt(startPos)>=0x01)){//首字是不是ASCII，如果是，则本次执行时print英文，否则print中文
			//Serial.print("Try English.");
			for(; (text.charAt(endPos)<0x80)&&(text.charAt(endPos)>=0x01); endPos++);//要排除\0
			//Serial.print("@");
			tft.print(text.substring(startPos, endPos));
			//Serial.print(text.substring(startPos, endPos));
			//startPos = endPos;
		}else if(text.charAt(startPos)>=0x80){
			//Serial.print("Try Chinese.");
			for(; text.charAt(endPos)>=0x80; endPos++);
			//Serial.print(text.substring(startPos, endPos));
			gbkBuf = new uint16_t[endPos-startPos];
			chineseLen = Utf8ToGb2312(text.substring(startPos, endPos).c_str(), endPos-startPos, gbkBuf);
			printGBK(tft, gbkBuf, chineseLen, textColor, bgColor);
			delete gbkBuf;
			//Serial.print("$");
		}else{
			endPos++;
		}
		//endPos++;
		
	//}while(endPos < textLen);
	if(endPos>=textLen){
		return;
	}else{
		return printChineseString(tft, text.substring(endPos,textLen), textColor, bgColor);
	}
}
