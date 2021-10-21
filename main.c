//Juan Carlos Ortiz de Montellano Bochelen
//A01656520
//Actividad-7
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#define N 20 //numero de secciones por las cuales deben pasar los pedidos
#define PesoMaximoSeccion 20 //Peso maximo que soportan las secciones
#define PesoRobotGlobal 2 //El peso inicial del robot sin ningun pedido encima

//Constantes
const useconds_t tiempoMaximo = 10;
const useconds_t tiempoMinimo = 30;
const size_t cantidadMaximaSecciones = 5;
const size_t cantidadRobots = 6;
const int productos = 3;
const int pesoMaximoP =10;
size_t cantidadSecciones;

//Hilos
pthread_cond_t variableCondicion = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex  = PTHREAD_MUTEX_INITIALIZER;

//struct de nuestra seccion
struct seccion* laSeccion;

struct seccion{
    useconds_t tiempoFinal;
    int capacidadMaxima;
    int pesoActual;
    int pesoProducto;

}
//Metodos
//#========================================================
//Metodos de inicializacion

//aqui iniciaremos nuestro struct
linkStart(void* inicio){
    struct seccion* end = (struct seccion*) inicio;
    //definiremos los datos iniciales de nuestro struct
    end->pesoActual =0;//inicializamos en 0
    end->tiempoFinal=rand()%tiempoMinimo+tiempoMaximo; //generaremos el tiempo de despache aleatoriamente
    end->pesoProducto=rand() % 1+(pesoMaximoP);//generaremos valores aleatorios para el peso de nuestro producto
    end->capacidadMaxima=rand() % ((pesoMaximoP*productos)-(2*pesoMaximoP)+1)+(2*pesoMaximoP); //generaremos la capacidad maxima de manera aleatoria
}


void robot(void*args){
    //variables de nuestro robot
    //identificador de nuestro robot
    int numeroRobot =rand()%100;
    //nuestra lista de secciones para el robot
    int pedido [cantidadSecciones];
    //peso inicial de nuestro robot
    int peso=0;

    //mandar a llamar a los metodos
    obtenerLista(pedido);
    //tiempo espera
    usleep(100);

    for (int numeroRobot = 0; cantidadSecciones > numeroRobot; numeroRobot++)
    {
        int compra = obtenerProductos(numeroRobot,peso,pedido[numeroRobot]);
        int in =0;
        while (!in)
        {
            pthread_mutex_lock(&mutex);
            struct seccion*laSeccion = laSeccion+numeroRobot;
            int after = laSeccion->pesoActual + laSeccion->pesoProducto * compra +peso;
            int allow = laSeccion->capacidadMaxima >= after;
            if (allow)
            {
                //se mandara a liberar la seccion
                liberar(numeroRobot,peso);
                //se actualizara el valor del peso por el nuevo
                laSeccion->pesoActual = after; 
                //modificamos el valor
                in=1;
                //desplegamos los valores
                printf("Nuestro robot se encuentra en la seccion:");
                printf("%d",numeroRobot+1);
                printf("y su peso actual es de:");
                printf("%d",laSeccion->pesoActual);
                //manejo de hilos
                pthread_cond_broadcast(&variableCondicion);
                pthread_mutex_unlock(&mutex);
                //se manda a llamar a una nueva funcion
                tiempoEspera(compra,&peso,laSeccion);
            }else
            {
                //bloquearemos nuestro hilo
                pthread_mutex_lock(&mutex);
                liberar(cantidadSecciones,peso);
                pthread_cond_broadcast(&variableCondicion);
                //desbloquearemos nuestro hilo
                pthread_mutex_unlock(&mutex);
                //terminaremos nuestro proceso con el hilo y saldremos 
                pthread_exit(NULL);
            }
            
            
        }
        
    }
    


}
//#========================================================


//#========================================================
//Metodos para las secciones
//Aqui generaremos nuestras secciones 
void generarSeccion(){
    int contador=1;
    cantidadSecciones = random()% 1+cantidadMaximaSecciones;
    size_t tamanoSecciones = sizeof(struct seccion) * cantidadSecciones;
    laSeccion = (struct seccion*) malloc(tamanoSecciones);
    struct seccion* end = cantidadSecciones + laSeccion;
    //Se imprimiran los datos 
    for (struct seccion* laSeccion = laSeccion; laSeccion < end; laSeccion++)
    {
        linkStart(end);
        printf("La seccion %d",contador);
        printf(" tiene una capacidad maxima de:");
        printf("%d",laSeccion->capacidadMaxima); //referenciamos al atributo de nuestro struct
        printf("y el peso de los productos es de: ");
        printf("%d",laSeccion->pesoProducto); //referenciamos al atributo de nuestro struct
        contador++;
    }
    
}
//#========================================================



//#========================================================
//Metodos para las listas
//obtendremos nuestra lista de productos
void obtenerLista(int* lista){
    int* ultimoElemento = lista+cantidadSecciones;
    for (int* campo = lista; campo < ultimoElemento; campo++)
    {
        int shopingList = rand() % 0+productos;//generaremos los productos que compraremos 
        *campo = shopingList;
    }
    
}
//#========================================================



//#========================================================
//Metodos para los productos
//aqui obtendremos los datos de nuestros preoductos que seran comprados cuando se procesen
int obtenerProductos(int indice, int pesoRobot, int listaCompras){
    int pesoPermitido = (laSeccion+indice)->capacidadMaxima;
    for (int i = indice+1; i < cantidadSecciones; i++)
    {
        //se controlara el peso de nuestra seccion 
        if (pesoPermitido >(laSeccion+i)->capacidadMaxima)
        {
            pesoPermitido = (laSeccion+i)->capacidadMaxima;
        }
        
    }
    struct seccion*laSeccion=(struct seccion*)(laSeccion + indice);
    int pp = pesoRobot;
    //esta es la cantidad de productos que ira incrementando
    int pa=0;

    while (pa<listaCompras && pesoPermitido >= pp+laSeccion->pesoProducto)
    {
        pa+=1;
        pp+=laSeccion->pesoProducto;

    }
    return pa; 
    
}

//#========================================================

//#========================================================
//Metodos controladores
//en este metodo liberaremos la seccion previa de nuestro programa
void liberar(int seccionLiberada, int pR){
    if (seccionLiberada==0)
    {
        printf("");
    }else
    {
        seccionLiberada--;
        struct seccion*laSeccion = seccionLiberada+laSeccion;
        laSeccion->pesoActual -= pR;
    }
}

//aqui se manejara el tiempo de espera para la recepcion de nuestros productos
void tiempoEspera(int cantidadFinal, int*pRobot, void*secundario){
    struct seccion*laSeccion = (struct seccion*)secundario;
    *pRobot += cantidadFinal * laSeccion->pesoProducto;
    useconds_t wait = cantidadFinal * laSeccion->tiempoFinal;
    printf("Ahora se esta esperando");
    usleep(wait);
}

//#========================================================
//Metodo main
//Aqui se ejecutara nuestro programa por primera vez

int main()
{
    generarSeccion();
    //crearemos un numero de robots aleatorio
    const int num_robots = rand() %1 +cantidadRobots;
    pthread_t bots[num_robots];
    pthread_t* finbot = bots + num_robots;

    for (pthread_t* bot = bots; finbot > bot; bot++)
    {
        //creacion de hilos
        pthread_create(bot,NULL,(void*)robot,NULL);
    }
    for (pthread_t* bot = bots; finbot > bot; bot++)
    {
        //unirse a los hilos
        pthread_join(*bot,NULL);
    }
    //limpiar
    free(laSeccion);
    return 0;
}

