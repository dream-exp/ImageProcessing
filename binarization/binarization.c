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
  if(argc != 4) {
    printf("Usage : ./binarization <binarization type> <input pgm filename> <output pgm filename>\n");
    exit(0);
  } else if (argc == 4) {
    image_open(argv[2]);
    printf("mode : %s\n", argv[1]);
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

  /* ヒストグラム生成部 */
  int histogram[max_value];
  for(int i=0; i <= max_value; i++)
    histogram[i] = 0; //ヒストグラム用配列の初期化

  for(int i=0; i < width; i++) {
    for(int j=0; j < height; j++) {
      histogram[image[j][i]]++;
    }
  }

  // for(int i=0; i <= max_value; i++)
  //   printf("i=%d, v=%d\n", i, histogram[i]);

  /* 閾値計算部 */

  int threshold = 0;

  int output_image[width][height];

  for(int i=0; i < height; i++) {
    for(int j=0; j < width; j++) {
      output_image[j][i] = image[j][i];
    }
  }

  if(strcmp(argv[1], "mode") == 0) { //モード法

    int dHist[max_value-1]; //微分ヒストグラム用の配列
    for(int i=0; i <= (max_value-1); i++) {
      dHist[i] = histogram[i] - histogram[i+1]; //微分ヒストグラム用配列の計算
      // printf("i=%d, dHist[i]=%d \n", i, dHist[i]);
    }

    int range = 40; //最頻値検出用の微分ヒストグラムの探索幅
    int valley = 0; //谷の画素値
    int valley_first = 1;

    for(int i = range; i < (max_value - range); i++) {
      int minus_range = 0;
      int plus_range = 0;

      for(int j = 0; j <= range; j++) { //両側レンジの微分ヒストグラムの合計値の計算
        minus_range = minus_range + dHist[i-j];
        plus_range = plus_range + dHist[i+j];
      }

      printf("i = %d, minus_range = %d, plus_range = %d\n", i, minus_range, plus_range);

      if(minus_range >= 0 && plus_range <= 0) { //マイナス側レンジが右下がりの傾き かつ プラス側レンジが右上がりの傾きのとき
        if(valley_first == 1 || histogram[valley] > histogram[i]) //現在のヒストグラムの値がvalleyより低ければvalleyを更新
          valley = i;
          if(valley_first == 1)
            valley_first = 0;
      }
    }

    printf("valley=%d\n", valley);

    threshold = valley;

    for(int k=0; k <= max_value; k++) {
      // printf("%4d %5d\n", k, histogram[k]);
    }

    // int padding = ((kernel_size - 1)/2);
    // printf("padding:%d\n", padding);
    //
    // int i, j;
    //
    // for(i=padding; i < (height-padding); i++) {
    //   for(j=padding; j < (width-padding); j++) {
    //     int sum = 0;
    //
    //     for(int k = -padding; k <= padding; k++) {
    //       for(int l = -padding; l <= padding; l++) {
    //         sum += image[j+k][i+l];
    //       }
    //     }
    //     output_image[j][i] = (int)(sum / (kernel_size * kernel_size));
    //   }
    // }

  } else if(strcmp(argv[1], "otsu") == 0) { //判別分析法（大津の2値化）

    int min = -1; //最小値
    int max = -1; //最大値
    int sum = 0;

    for(int i = 0; i <= max_value; i++) { //平均値、最小値、最大値を求める
      sum += (histogram[i] * i);

      if(histogram[i] != 0) { //その輝度値に画素が存在している場合
        if(min == -1)
          min = i;

        max = i;
      }
    }

    if(min == -1 || max == -1)
      exit(-1);

    float ave = sum / (width * height);
    printf("Average : %f, Max : %d, Min : %d\n", ave, max, min);

    /* 分離度Sを求め、閾値を決める */
    int s = 0; //分離度S

    for(int t = min; t < max; t++) {
      int n1 = 0; //tより左側の画素数の合計
      int v1 = 0; //tより左側の画素値の合計
      int n2 = 0; //tより右側の画素数の合計
      int v2 = 0; //tより右側の画素値の合計

      for(int j = min; j <= max; j++) {
        if(j <= t) {
          n1 += histogram[j];
          v1 += (histogram[j] * j);
          // printf("j : %d, n1 : %d, v1 : %d\n", j, n1, v1);
        } else {
          n2 += histogram[j];
          v2 += (histogram[j] * j);
          // printf("j : %d, n2 : %d, v2 : %d\n", j, n2, v2);
        }
      }

      // printf("t : %d, n1 : %d, v1 : %d\n", t, n1, v1);
      // printf("t : %d, n2 : %d, v2 : %d\n", t, n2, v2);
      // printf("n1+n2 %d\n", n1+n2);

      float ave1 = (float)(v1 / n1); //tより左側の画素値の平均
      float ave2 = (float)(v2 / n2); //tより右側の画素値の平均

      // printf("t : %d, ave1 : %f, ave2 : %f\n", t, ave1, ave2);

      float sum1 = 0; //tより左側の平均との差の二乗を足し込む変数
      float sum2 = 0; //tより右側の平均との差の二乗を足し込む変数

      for(int j = min; j <= max; j++) {
        if(j <= t)
          sum1 += ((j - ave1) * (j - ave1));
        else
          sum2 += ((j - ave2) * (j - ave2));
      }

      float sigma_1 = sum1 / n1; //tより左側の分散
      float sigma_2 = sum2 / n2; //tより右側の分散

      float sigma_w = (n1 * sigma_1 + n2 * sigma_2) / (n1 + n2); //クラス内分散
      float sigma_b = (n1 * ((ave1 - ave) * (ave1 - ave)) + n2 * ((ave2 - ave) * (ave2 - ave))) / (n1 + n2); //クラス外分散

      float s_local = sigma_b / sigma_w; //分離度の計算
      // int s_local = n1 * n2 * (ave1 - ave2) * (ave1 - ave2);

      if(s_local > s) {//分離度が大きければ閾値を更新
        s = s_local;
        threshold = t;
      }
    }
  }

  printf("threshold : %d\n", threshold);
  /* 2値化処理 */

  for(int i = 0; i < height; i++) {
    for(int j = 0; j < width; j++) {
      if(image[j][i] > threshold)
        output_image[j][i] = max_value;
      else
        output_image[j][i] = 0;
    }
  }

  /* 画像出力部 */

  FILE *output = fopen(argv[3], "wb");
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
