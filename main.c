#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <pthread.h>
#include <string.h>

#define NUM_THREADS 4


typedef struct {
    int  len;
    char** paths;
} ImagesResponse;

int image_filter(const struct dirent *entry) {
    // Obtener la extensión del archivo
    const char *ext = strrchr(entry->d_name, '.');

    // Verificar si la extensión corresponde a una imagen
    if (ext && (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".png") == 0 ||
                strcmp(ext, ".jpeg") == 0 || strcmp(ext, ".bmp") == 0)) {
        return 1;  // Incluir este archivo
                }
    return 0;  // Excluir este archivo
}

void get_images(const char *path, ImagesResponse *response) {
    int i = 0;
    struct dirent **files;


    response->len = scandir(path,&files,image_filter,alphasort);


    if(response->len < 0){
        perror("no fue posible leer el directorio");
        return;
    }

    // se instancia el arreglo para guardar punteros de strings
    response->paths = malloc((response->len+1) * sizeof(char*));


    for(i = 0; i < response->len; i++){
        char currentWord[250];
        struct dirent *entry = files[i];
        switch (entry->d_type) {
            case DT_REG:
                sprintf(currentWord,"%s\n", entry->d_name);
            break;

            case DT_DIR:
                sprintf(currentWord,"%s/\n", entry->d_name);
            break;

            case DT_LNK:
                sprintf(currentWord,"%s@\n", entry->d_name);
            break;

            default:
                sprintf(currentWord,"%s*\n", entry->d_name);
        }
        // reservamos el espacio de la cadena
        // sobre el puntero de string se reserva la memoria para el string
        response->paths[i] = malloc(strlen(currentWord) + 1);
        strcpy(response->paths[i], currentWord); // se copia el string
        free(entry);
    }

}

int main(){

    ImagesResponse response;
    get_images("/home/sistemas/Imágenes",&response);
    for(int i = 0; i < response.len; i++) {
        printf("%s",response.paths[i]);
    }

    return 0;
}