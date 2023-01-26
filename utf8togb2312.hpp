//#include <stdio.h>
//#include <string.h>
//#include "cp936.hpp"
#include <pgmspace.h>
#include "utf16.h"
/*
#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int*/

int GetUtf8ByteNumForWord(uint8_t firstCh)
{
  uint8_t temp = 0x80;
  int num = 0;

  while (temp & firstCh)
  {
		num++;
		 temp = (temp >> 1);
   }
	//Serial.printf("the num is: %d\n", num);
	return num;
}


/*uint16_t SearchCodeTable(uint16_t unicodeKey)
{
    int first = 0;
    int end = CODE_TABLE_SIZE - 1;
    int mid = 0;
    while (first <= end)
    {
        mid = (first + end) / 2;
        if (code_table[mid].unicode == unicodeKey)
        {
            return code_table[mid].gb;
        }
        else if (code_table[mid].unicode > unicodeKey)
        {
            end = mid - 1;
        }
        else 
        {
            first = mid + 1;
        }
    }
    return 0xA1F5;//□
}*/

uint16_t SearchCodeTable(uint16_t unicodeKey){//用于读取utf16.bin
	int first = 0;
	int end = CODE_TABLE_SIZE - 1;//这个文件的CODE_TABLE_SIZE是6963
	int mid = 0;
	uint16_t unicode;
	while (first <= end){
		mid = (first+end)/2;
		unicode = (pgm_read_byte(code_table+mid*4+1)<<8)|pgm_read_byte(code_table+mid*4);
		if(unicode == unicodeKey){
			return (pgm_read_byte(code_table+mid*4+2)<<8)|pgm_read_byte(code_table+mid*4+3);
		}else if(unicode > unicodeKey){
			end = mid - 1;
		}else{
			first = mid + 1;
		}
	}
	return 0xA1F5;//“□”，用于替换GB2312未收录汉字
}

uint32_t Utf8ToGb2312(const char* utf8, int len,uint16_t* gbArray)
{
	int k=0;
	int byteCount = 0;
	int i = 0;
	int j = 0;
	char* temp = new char[len];
	uint16_t unicodeKey = 0;
	uint16_t gbKey = 0;

	//循环解析
	while (i < len){   
		switch(GetUtf8ByteNumForWord((uint8_t)utf8[i])){
			case 0:
			temp[j] = utf8[i];
			byteCount = 1;
			break;

			case 1:
			//printf("utf8[i]=0x%X\n",utf8[i]);
			temp[j] = utf8[i];
			byteCount = 1;
			break;
			
			case 2:
			temp[j] = utf8[i];
			temp[j + 1] = utf8[i + 1];
			byteCount = 2;
			break;

			case 3:
			//这里就开始进行UTF8->Unicode
			temp[j + 1] = ((utf8[i] & 0x0F) << 4) | ((utf8[i + 1] >> 2) & 0x0F);
			temp[j] = ((utf8[i + 1] & 0x03) << 6) + (utf8[i + 2] & 0x3F);

			//取得Unicode的值
			memcpy(&unicodeKey, (temp + j), 2);

			//根据这个值查表取得对应的GB2312的值
			gbKey = SearchCodeTable(unicodeKey);
			//printf("gbKey=0x%X\n",gbKey);
			gbArray[k++]=gbKey;
			byteCount = 3;
			break;

			case 4:
			byteCount = 4;
			break;
				
			case 5:
			byteCount = 5;
			break;
				
			case 6:
			byteCount = 6;
			break;

			default:
			Serial.printf("the len is more than 6\n");
			return 0;
			break;    
		}
		i += byteCount;
		if (byteCount == 1){
			j++;
		}else{
			j += 2;
		}
	}
	delete(temp);
	return k;
}

int mian(){
	int i,n;
	char str[]="人之初，性本善，性相近，习相远。";
	char s;
	int num=strlen(str);
	n=num/3;
	uint16_t gbArray[num];
	Utf8ToGb2312(str,num,gbArray);
	for(i=0;i<n;i++){
		//Serial.printf("%c%c%c：0x%X\n",str[3*i],str[3*i+1],str[3*i+2],gbArray[i]);
		s = gbArray[i]>>8;
		Serial.print(s);
		s = gbArray[i];
		Serial.print(s);
	}
	return 0;	
}
