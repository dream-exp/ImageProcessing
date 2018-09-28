#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

FILE *fp;

int width;
int height;
int max_value;

int status;
char buffer[128];

void image_open(char *file_name) {
  if ((fp = fopen(file_name, "r")) == NULL) {
		printf("file open error!!\n");
		exit(-1);	/* (3)エラーの場合は通常、異常終了する */
	}
  // printf("%s\n", file_name);
}

int main(int argc,char *argv[]) {

  /* 入力コマンドのチェック */
  if(argc == 1) {
    printf("Usage : ./histogram <pgm file name>\n");
    exit(0);
  } else if (argc == 2) {
    image_open(argv[1]);
  }

  /* ヘッダ取得部 */
  int ch;

  while (status < 3) {
    ch = getc(fp);

    if(ch == '#') { // コメントのスキップ
      while (( ch = getc(fp)) != '\n')
        break;
    }

    if(ch == 'P') { // マジックナンバーのチェック
      if(getc(fp) != '5') {
        printf("Magic Number is wrong.\n");
        break;
      } else {
        // printf("Magic Number is P5\n");
      }
    }

    if(isdigit((unsigned char)ch)) { // 数値取得部
      buffer[0] = ch;
      int i=1;
      while(1) {
          char c = getc(fp);
          if(isdigit((unsigned char)c)) {
            buffer[i]=c;
            i++;
          } else
            break;
      }
      buffer[i] = '\0';

      switch (status) {
        case 0: // width取得部
          width = atoi(buffer);
          // printf("width=%d\n", width);
          break;
        case 1: // height取得部
          height = atoi(buffer);
          // printf("height=%d\n", height);
          break;
        case 2: // max_value取得部
          max_value = atoi(buffer);
          // printf("max_value=%d\n", max_value);
          break;
      }
      status++; // 次のステータスへ
    }
  }

  /* ヒストグラム生成部 */
  int histogram[1024] = {};

  while ((ch=getc(fp)) != -1) {
    histogram[(int)ch]++;
  }

  for(int k=0; k <= max_value; k++) {
    printf("%4d %5d\n", k, histogram[k]);
  }
}
