#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
}

int main(int argc,char *argv[]) {

  /* 入力コマンドのチェック */
  if(argc != 5) {
    printf("Usage : ./filter <filter type> <kernel size> <input pgm filename> <output pgm filename>\n");
    exit(0);
  } else if (argc == 5) {
    image_open(argv[3]);
    printf("filter type : %s\n", argv[1]);
    printf("kernel size : %s\n", argv[2]);
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
          printf("width=%d\n", width);
          break;
        case 1: // height取得部
          height = atoi(buffer);
          printf("height=%d\n", height);
          break;
        case 2: // max_value取得部
          max_value = atoi(buffer);
          printf("max_value=%d\n", max_value);
          break;
      }
      status++; // 次のステータスへ
    }
  }

  /* 画素値取得部 */
  int image[width][height];

  for(int i = 0; i < height; i++) {
    for(int j = 0; j < width; j++) {
      image[j][i] = (int)getc(fp);
    }
  }

  /* フィルタ処理部 */

  int kernel_size = atoi(argv[2]);
  int output_image[width][height];

  for(int i=0; i < height; i++) {
    for(int j=0; j < width; j++) {
      output_image[j][i] = image[j][i];
    }
  }

  if(strcmp(argv[1], "average") == 0) { //加重平均フィルタ

    int padding = ((kernel_size - 1)/2);
    printf("padding:%d\n", padding);

    int i, j;

    for(i=padding; i < (height-padding); i++) {
      for(j=padding; j < (width-padding); j++) {
        int sum = 0;

        for(int k = -padding; k <= padding; k++) {
          for(int l = -padding; l <= padding; l++) {
            sum += image[j+k][i+l];
          }
        }
        output_image[j][i] = (int)(sum / (kernel_size * kernel_size));
      }
    }

  } else if(strcmp(argv[1], "median") == 0) { //メディアンフィルタ
    int padding = ((kernel_size - 1)/2);
    printf("padding:%d\n", padding);

    int i, j;

    for(i=padding; i < (height-padding); i++) {
      for(j=padding; j < (width-padding); j++) {
        int sum = 0;
        int median_array[kernel_size * kernel_size];
        int median_cnt = 0;

        for(int m = 0; m < kernel_size * kernel_size; m++)
          median_array[m] = 0;

        for(int k = -padding; k <= padding; k++) {
          for(int l = -padding; l <= padding; l++) {
            median_array[median_cnt] = image[j+k][i+l];
            median_cnt++;
          }
        }

        for(int m = 0; m < (kernel_size * kernel_size); m++) {
          for(int n = 0; n < (kernel_size * kernel_size); n++) {
                if (median_array[n+1] < median_array[n]) {
                    int buf = median_array[n+1];
                    median_array[n+1] = median_array[n];
                    median_array[n] = buf;
                }
            }
        }

        output_image[j][i] = (int)median_array[(kernel_size * kernel_size) / 2];
      }
    }
  } else if(strcmp(argv[1], "laplacian") == 0) { //ラプラシアンフィルタ
    if(kernel_size != 3) {
      printf("ラプラシアンフィルタを適用する場合はカーネルサイズを3にしてください\n");
      exit(0);
    }

    int padding = ((kernel_size - 1)/2);
    printf("padding:%d\n", padding);
    int laplacian[3][3] = {{1, 1, 1}, {1, -8, 1}, {1, 1, 1}};
    int i, j;

    for(i=padding; i < (height-padding); i++) {
      for(j=padding; j < (width-padding); j++) {
        int sum = 0;

        for(int k = -padding; k <= padding; k++) {
          for(int l = -padding; l <= padding; l++) {
            sum += (image[j+k][i+l] * laplacian[k+padding][l+padding]);
          }
        }
        if(sum < 0)
          output_image[j][i] = 0;
        else if(sum > max_value)
          output_image[j][i] = max_value;
        else
          output_image[j][i] = (int)sum;
      }
    }
  }

  /* 画像出力部 */

  FILE *output = fopen(argv[4], "wb");
  char header[30];
  sprintf(header, "P5 %d %d %d\n", width, height, max_value);
  fputs(header, output);
  for(int i = 0; i < height; i++) {
    for(int j = 0; j < width; j++) {
      fputc((char)output_image[j][i], output);
    }
  }
  printf("image was output.\n");
  fclose(output);
}
