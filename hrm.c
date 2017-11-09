#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

#define debug

#ifdef debug
#define dprint printf
#else
#define dprint(...) do {} while (0)
#endif

#define bufend (buf + buflen)
#define BUF_SIZE 128
#define LABEL_NUM 64
#define CARPET_SIZE 64

const char *kwtbl[] = {
	//"\n",
	"inbox",
	"outbox",
	"copyfrom",
	"copyto",
	"add",
	"sub",
	"bump+",
	"bump-",
	"jumpifzero",
	"jumpifneg",
	"jump",
	"[","]",
	//ここから独自のキーワード
	":",	//ラベル
	"get",	//即値
	"run"	//実行
};

const int KWTBL_SIZE = sizeof(kwtbl)/sizeof(char *);

enum {
	//i_newline,
	i_inbox,
	i_outbox,
	i_copyfrom,
	i_copyto,
	i_add,
	i_sub,
	i_bump_p,
	i_bump_m,
	i_jumpifzero,
	i_jumpifneg,
	i_jump,
	i_open,
	i_close,
	//ここから独自のキーワード
	i_label,	//ラベル
	i_get,	//即値
	i_run,
	//特集キーワード
	i_num,
	i_pass,
	i_char,
};

typedef struct
{
	int name;
	int* p;
} label;

enum {
	type_empty,
	type_num,
	type_char
};

typedef struct
{
	char type;
	int num;
} tile;

char raw[BUF_SIZE*16];

int buf[BUF_SIZE];
int buflen = 0;

label lblls[LABEL_NUM];
int labelpt = 0;

tile acm = {type_empty, 0};
tile carpet[CARPET_SIZE];

void init()
{
	for(int i=0;i<CARPET_SIZE;i++){
		carpet[i].type = type_empty;
		carpet[i].num = 0;
	}
}

void trans()
{
	char *rawp = raw;	//文字列走査位置
	int iword = 0;		//中間コード置き場
	bool is_find;

	while(*rawp){//文字列の終端まで
		is_find = false;
		while(isspace(*rawp)) rawp++;
		if(!*rawp) break;

		//数字変換
		if(isdigit(*rawp)||
		*rawp=='-'||*rawp=='+'){
			iword = atoi(rawp);
			buf[buflen++] = i_num;
			buf[buflen++] = iword;
			while(isdigit(*rawp)||
			*rawp=='-'||*rawp=='+')
				rawp++;
			continue;
		}

		//文字解決
		if(*rawp=='\''){
			char temp = *(++rawp);
			if(temp == '\\'){
				if(*(++rawp) == 'n')
					temp = '\n';
				else{
					printf("char error\n");
					return;
				}
			}
			if(*(++rawp) != '\''){	//終端確認
				printf("char error\n");
				return;
			}
			buf[buflen++] = i_char;
			buf[buflen++] = temp;
			rawp++;
			continue;
		}

		//テーブル走査
		for(iword=0;iword < KWTBL_SIZE;iword++)
		{
			int k = 0;
			while(kwtbl[iword][k]!=0 &&
				kwtbl[iword][k]==rawp[k])
				k++;
			if(kwtbl[iword][k]==0 &&
				!isalpha(rawp[k])){
				buf[buflen++] = iword;
				rawp += k;
				is_find = true;
				break;
			}
			dprint("i:%d\n",iword);
		}
		if(!is_find){
			printf("error find unkown word\n");
			return;//----------error---------->
		}
		//end---------
	}
	printf("ok\n");
}

void setlabel(int name, int* p)
{
	dprint("set %d\n",name);
	lblls[labelpt].name = name;
	lblls[labelpt].p = p;
	labelpt++;
}
int* labelsearch(int name)
{
	//dprint("search num%d\n",name);
	for(int i = 0;i < labelpt;i++)
	{
		if(lblls[i].name == name)
			return lblls[i].p;
	}
	printf("label error\nnumber:%d\n",labelpt);
	return NULL;
}

/*  old
void input()
{
	fgets(buf, BUF_SIZE, stdin);
	//bufend = buf + BUF_SIZE;
}
*/

void setup()
{
	int* bufp = buf;
	int lbname;
	for(;bufp < bufend;bufp++)
	{
		if(*bufp == i_label)
		{
			if(*(bufp+1) != i_num){
				printf("no name label\n");
				return;//---------error------->
			}
			bufp += 2;
			lbname = *bufp;
			*(bufp-2)= *(bufp-1)= *bufp=i_pass;
			//^i_label   ^i_num    ^label name
			setlabel(lbname, bufp);
		}
	}
}

void reader()
{
	int* bufp = buf;
	while(bufp < bufend)
	{
		switch(*bufp)
		{
		case i_outbox:
			switch (acm.type) {
				case type_empty:
					printf("error nothing to outbox\n");
					return;
				case type_char:
					putchar(acm.num);
					break;
				case type_num:
					printf("%d\n",acm.num);
					break;
			}
			//printf("!outbox\n");
			break;
		case i_get:
			switch(*(++bufp)){
				case i_num:
					acm.type = type_num;
					acm.num = *(++bufp);
					break;
				case i_char:
					acm.type = type_char;
					acm.num = *(++bufp);
					break;
				default:
					printf("get error\n");
					return;
			}
			break;
		//jump----------------
		case i_jumpifneg:
		//原作の仕様通り、型チェックはしない
		//空のときのみ例外
			if(acm.type == type_empty){
				printf("if error no tile\n");
				return;
			}
			if(acm.num < 0){
				bufp += 2;
				int* p = labelsearch(*bufp);
				if(p == NULL)
					return;
				bufp = p;
			}
			break;
		case i_jumpifzero:
		//原作の仕様通り、型チェックはしない
		//空のときのみ例外
			if(acm.type == type_empty){
				printf("if error no tile\n");
				return;
			}
			if(acm.num==0){
				bufp += 2;
				int* p = labelsearch(*bufp);
				if(p == NULL)
					return;
				bufp = p;
			}
			break;
		case i_label:
			printf("?\n");
		case i_jump:
			bufp += 2;
			int* p = labelsearch(*bufp);
			if(p == NULL)
				return;
			bufp = p;
			break;
		//jump end --------------
		}
		//dprint("iok\n");
		bufp++;
	}
}

void run()
{
	setup();
	reader();
}

int main()
{
	init();
	bool run_start = false;
	dprint("table size: %d\n",KWTBL_SIZE);
	for(;;)
	{
		putchar('>');
		fgets(raw, 256, stdin);
		trans();
		run_start = false;
		dprint("-----------\n");
		for(int i=0;i<buflen;i++){
			if(buf[i] == i_run){
				buflen = i;
				run_start = true;
				break;
			}
			#ifdef debug
			if(buf[i] == i_num){
				printf("i_num\n");
				printf("%d\n",buf[++i]);
				continue;
			}
			if(buf[i] == i_char){
				printf("i_char\n");
				printf("%c\n",buf[++i]);
				continue;
			}
			if(buf[i] == i_pass){
				printf("i_pass\n");
				continue;
			}
			if(0<buf[i] && buf[i]<KWTBL_SIZE)
				printf("%s\n",kwtbl[buf[i]]);
			#endif
		}
		dprint("-----------\n");
		if(run_start){
			printf("start!\n");
			run();
		}
	}
	return 0;
}
