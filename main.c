#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#pragma pack(push, 1)

struct header {
    unsigned char tag[2];
    int fileSize;
    short reserved1;
    short rReserved2;
    int offBits;
    unsigned int headerSize;
    int picWidth;
    int picHeight;
    short planes;
    short biBitCount;
    unsigned int compression;
    unsigned int picSize;
    unsigned int verticalResolution;
    unsigned int horizontalResolution;
    unsigned int clrUsed;
    unsigned int colours;
    int color_table[2];
} header;

int main(int argc, char *argv[]) {

    char *input = NULL;
    char *output = NULL;
    int max_iter = 20;
    int dump_freq = 1;
    if (argc > 4) {
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], "--input") == 0) {
                input = argv[i + 1];
            } else if (strcmp(argv[i], "--output") == 0) {
                output = argv[i + 1];
            } else if (strcmp(argv[i], "--max_iter") == 0) {
                max_iter = atoi(argv[i + 1]);
            } else if (strcmp(argv[i], "--dump_freq") == 0) {
                dump_freq = atoi(argv[i + 1]);
            }
        }
    } else {
        printf("Invalid data, not enough arguments");
        exit(1);
    }

    if (input == NULL) {
        printf("Invalid data, you didn't enter source BMP file");
        exit(1);
    } else if (output == NULL) {
        printf("Invalid data, you didn't enter filepath for BMP output");
        exit(1);
    } else if (max_iter < 1) {
        printf("Invalid data, too few generations, minimum: 1");
        exit(1);
    } else if (dump_freq < 1) {
        printf("Invalid data, too low frequency, minimum: 1");
        exit(1);
    }


    FILE *f1;
    f1 = fopen(input, "rb");
    fread(&header, sizeof(header), 1, f1);

    if (header.tag[0] != 'B' && header.tag[1] != 'M') {
        printf("Invalid file format, required: bmp");
    }
    if (header.biBitCount != 8) {
        printf("Invalid color depth, required depth: 8 bit/pixel");
        exit(1);
    }

    int height = header.picHeight;
    int width = header.picWidth;
    unsigned char **pic = calloc(height, sizeof(char *));
    for (int i = 0; i < height; i++) {
        pic[i] = calloc(width, sizeof(char));
    }

    int trash = 4 - width % 4;
    for (int i = 0; i < height; i++) {
        fread(pic[i], sizeof(char) * width, 1, f1);
        fseek(f1, trash, SEEK_CUR);
    }

    unsigned char **newPic = calloc(height, sizeof(char *));
    for (int i = 0; i < height; i++) {
        newPic[i] = calloc(width, sizeof(char));
    }

    for (int iter = 0; iter < max_iter; iter++) {

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int sum;
                if (j == 0) {
                    if (i == 0) {
                        sum = pic[i][j] + pic[i][j + 1] + pic[i + 1][j] + pic[i + 1][j + 1];
                    } else if (i == height - 1) {
                        sum = pic[i - 1][j] + pic[i][j + 1] + pic[i - 1][j + 1];
                    } else {
                        sum = pic[i - 1][j] + pic[i - 1][j + 1] + pic[i][j + 1] + pic[i + 1][j + 1] + pic[i + 1][j];
                    }
                } else if (j == width - 1) {
                    if (i == 0) {
                        sum = pic[i][j - 1] + pic[i + 1][j] + pic[i + 1][j - 1];
                    } else if (i == height - 1) {
                        sum = pic[i - 1][j] + pic[i][j - 1] + pic[i - 1][j - 1];
                    } else {
                        sum = pic[i - 1][j] + pic[i - 1][j - 1] + pic[i][j - 1] + pic[i + 1][j - 1] + pic[i + 1][j];
                    }
                } else {
                    if (i == 0) {
                        sum = pic[i][j - 1] + pic[i + 1][j - 1] + pic[i + 1][j] + pic[i + 1][j + 1] + pic[i][j + 1];
                    } else if (i == height - 1) {
                        sum = pic[i][j - 1] + pic[i - 1][j - 1] + pic[i - 1][j] + pic[i - 1][j + 1] + pic[i][j + 1];
                    } else {
                        sum = pic[i][j - 1] + pic[i - 1][j - 1] + pic[i - 1][j] + pic[i - 1][j + 1] + pic[i][j + 1] +
                              pic[i + 1][j + 1] + pic[i + 1][j] + pic[i + 1][j - 1];
                    }
                }
                if (pic[i][j] == 0) {
                    if (sum == 3) {
                        newPic[i][j] = 1;
                    } else {
                        newPic[i][j] = 0;
                    }
                } else {
                    if (sum == 2 || sum == 3) {
                        newPic[i][j] = 1;
                    } else {
                        newPic[i][j] = 0;
                    }
                }
            }
        }
        char *newOutput = malloc(10 * sizeof(char));
        strcpy(newOutput, output);
        char *iterToChar = malloc(10 * sizeof(char));
        sprintf(iterToChar, "%d", iter + 1);
        newOutput = strcat(newOutput, iterToChar);
        newOutput = strcat(newOutput, " generation");
        newOutput = strcat(newOutput, ".bmp");

        sleep(dump_freq);

        FILE *f2 = fopen(newOutput, "wb");
        fwrite(&header, sizeof(header), 1, f2);
        for (int i = 0; i < height; i++) {
            fwrite(newPic[i], width, 1, f2);
            fwrite("0", sizeof(char), trash, f2);
        }

        fclose(f2);
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                printf("%d ", newPic[i][j]);
            }
            printf("\n");
        }
        printf("\n");

        memcpy(*pic,*newPic, header.picWidth * header.picHeight * sizeof(char));
    }


    return 0;
}

#pragma pack(pop)
