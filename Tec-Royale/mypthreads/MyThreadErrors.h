#ifndef _MYTHREADERRORS_H
#define	_MYTHREADERRORS_H

enum ThreadErros
{
    SUCESS = 0, //El valor que se devuelve si todo salio bien
    INVALID_OPERATION = -1, //Problema con los parametros dados.
    MY_THREAD_NOT_INITIALIZED = -2, //Se devuelve cuando no se ha llamado primero a "my_thread_init"
    NOT_ENOUGH_MEMORY = -3, //Cuando no hay memoria suficiente para realizar la funcion.
    INVALID_THREAD_T = -4, //Cuando el parametro dado de "thread_t" es invalido.
    TRYLOCK_FAILED = -5,  //Se retorna si es no se pudo adquirir un lock
};

#endif