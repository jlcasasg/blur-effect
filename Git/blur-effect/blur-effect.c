//This program uses OpenCV.3.2.0
#include <stdio.h>
#include<opencv2/highgui/highgui.hpp>
#include<math.h>

#define PI 3.141592653589793238462643383
#define e  2.718281828459045235360287471


void gaussian_matrix(float ** matrix, int kernel){
    float sigma = 5.7;
    float value_x = floor(kernel/2)*-1;
    float value_y = floor(kernel/2);
    float constant = 1/(2*PI*(sigma*sigma));
    float sum = 0;
    
    for(int i=0; i < kernel; i++){        
        for(int j=0; j < kernel; j++){ 
            //Calculamos el valor de (1/2+pi*sigma)*e⁻(x²+y²/2*sigma²) 
           matrix[i][j] = constant * pow(e, -(((value_x*value_x)+ (value_y*value_y))/(2*sigma*sigma)));    
           value_x++;
           if(value_x==floor(kernel/2)+1){
               value_x = floor(kernel/2)*-1;
            }
            sum += matrix[i][j];    
        }        
        value_y--;
    }
    //Para que la suma de la matriz de 1 la normalizamos
    for(int i=0; i<kernel; i++){
        for(int j=0; j<kernel; j++){        
           matrix[i][j] = matrix[i][j] / sum;
        }
    }
}


int main(int args, char *argv[]){
    char *address = argv[1];
    int kernel = atoi(argv[2]);
    if(kernel%2==0){
        printf("kernel must be odd\n");
        return 0;
    }
    float** gaussian;
    gaussian = (float**) malloc(kernel * sizeof(float *) );
    //Creamos el espacio en memoria de la matriz gaussiana
    for(int i=0; i< kernel; i++){
        gaussian[i] = (float *) malloc(kernel * sizeof(float));
    }
    if(gaussian == NULL){
        printf("No memory space");
    }
    gaussian_matrix(gaussian, kernel);
    //Llamamos la imagen que queremos usar
    IplImage *img = cvLoadImage(address,CV_LOAD_IMAGE_COLOR);
    IplImage *result = cvCloneImage(img);
    
    int img_width = img->width;
    int img_height = img->height;
    
    int k = kernel/2;
    for(int i=0; i<img_height; i++){       
        for (int j=0; j<img_width; j++){
            CvScalar p, s;           
            double blue=0.0, red=0.0, green=0.0, neighbours = 0.0;
            for(int x=-k; x<=k; x++){
                for(int y=-k; y<=k; y++){             
                    int pos_x, pos_y;
                    if(i+x<0) pos_x = i+x*-1;
                    else if(i+x>=img_height) pos_x = i-x;
                    else pos_x = i+x;
                    if(j+y<0) pos_y = j+y*-1;
                    else if(j+y>=img_width) pos_y = j-y;
                    else pos_y = j+y;
                    
                    s = cvGet2D(img,pos_x,pos_y);
                    
                    blue += s.val[0]*gaussian[y+k][x+k];
                    green += s.val[1]*gaussian[y+k][x+k];
                    red += s.val[2]*gaussian[y+k][x+k];
                }      
            }
            p = cvGet2D(img,i,j);
            p.val[0] = blue;
            p.val[1] = green;
            p.val[2] = red;
            
            cvSet2D(result,i,j,p);
        }   
    }   
    cvNamedWindow("Image Filtered",CV_WINDOW_NORMAL);
    cvShowImage("Image Filtered",result);
    cvWaitKey(0);   
    //Liberamos memoria de la matriz gaussiana
    for(int i=0; i<kernel;i++){
        float *cur = gaussian[i];
        free(cur);
    }
    free(gaussian);
    return 0;
}