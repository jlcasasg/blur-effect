//This program uses OpenCV.3.2.0
#include <stdio.h>
#include <opencv2/highgui/highgui.hpp>
#include <math.h>
#include <pthread.h>

#define PI 3.141592653589793238462643383
#define e  2.718281828459045235360287471
#define MAX_THREADS 16
#define sigma  1.9

IplImage *result;
IplImage *img;
int kernel;
float **gaussian;
pthread_t th_id[MAX_THREADS];

struct Params{
    int begin;
    int end;  
}parameters[MAX_THREADS];

void gaussian_matrix(){
    
    float sum = 0;
    for(int i=0; i < kernel; i++){        
        for(int j=0; j < kernel; j++){ 
            //Calculamos el valor de (1/2+pi*sigma)*e⁻(x²+y²/2*sigma²) 
           gaussian[i][j] = (1/(2*PI*(sigma*sigma))) * pow(e, -(pow(floor(kernel/2),2)+ pow(floor(kernel/2),2)))/(2*sigma*sigma);    
           sum += gaussian[i][j];    
        }        
    }
    //Para que la suma de la matriz de 1 la normalizamos
    for(int i=0; i<kernel; i++){
        for(int j=0; j<kernel; j++){        
           gaussian[i][j] = gaussian[i][j] / sum;
        }
    }
}

void* blur(void *arguments){
    struct Params *param;
    param = arguments;
    int k = kernel/2;
    //Recorremos la matriz de la imagen original
    for(int i=param->begin; i<param->end; i++){       
        for (int j=0; j<img->width; j++){
            CvScalar p, s;           
            double blue=0.0, red=0.0, green=0.0;
            for(int x=-k; x<=k; x++){
                for(int y=-k; y<=k; y++){             
                    int pos_x, pos_y;
                    if(i+x<0) pos_x = i+x*-1;
                    else if(i+x>=img->height) pos_x = i-x;
                    else pos_x = i+x;
                    if(j+y<0) pos_y = j+y*-1;
                    else if(j+y>=img->width) pos_y = j-y;
                    else pos_y = j+y;          
                    //Obtenemos la posicion del pixel                                                            
                    s = cvGet2D(img,pos_x,pos_y);                    
                    blue += s.val[0]*gaussian[y+k][x+k];
                    green += s.val[1]*gaussian[y+k][x+k];
                    red += s.val[2]*gaussian[y+k][x+k];
                }      
            }
            //Modificamos la posicion del pixel de la imagen clonada
            p = cvGet2D(result,i,j);
            p.val[0] = blue;
            p.val[1] = green;
            p.val[2] = red;
            
            cvSet2D(result,i,j,p);
        }   
    }
    pthread_exit(NULL);
}

int main(int args, char *argv[]){
    char *address = argv[1];
    kernel = atoi(argv[2]);
    int n_threads = atoi(argv[3]);
    if(kernel%2==0){
        printf("kernel must be odd\n");
        return 0;
    }
    img = cvLoadImage(address,CV_LOAD_IMAGE_COLOR);
    //Creamos el espacio en memoria de la matriz gaussiana
    gaussian = (float**) malloc(kernel * sizeof(float *) );    
    for(int i=0; i< kernel; i++){
        gaussian[i] = (float *) malloc(kernel * sizeof(float));
    }
    if(gaussian == NULL){
        printf("No memory space");
    }
    gaussian_matrix();
    //Clonamos la imagen del resultado
    result =  cvCloneImage(img);
    
    //Creamos los hilos
    for(int i=0; i<n_threads; i++){    
        parameters[i].begin = i * (img->height/n_threads);
        parameters[i].end = parameters[i].begin + (img->height/n_threads)-1;      
    }
    
    for(int i=0; i<n_threads; i++){
        if(pthread_create(&th_id[i], NULL, blur, (void *)&parameters[i])!=0)
            perror("Thread could not be created");        
    }
    
    for(int i=0; i<n_threads; i++){
        if( pthread_join(th_id[i], NULL) != 0)
            perror("Thread could not end");
    }
    //Mostramos la imagen con el filtro
    cvNamedWindow("Image Filtered",CV_WINDOW_NORMAL);
    cvShowImage("Image Filtered", result);
    cvWaitKey(0);   
    //Liberamos memoria de la matriz gaussiana
    for(int i=0; i<kernel;i++){
        float *cur = gaussian[i];
        free(cur);
    }
    free(gaussian); 
    
    return 0;
}