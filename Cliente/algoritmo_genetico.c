#include <stdio.h>
#include <stdlib.h>
#include "algoritmo_genetico.h"
#include <time.h>
#include <math.h>
#include <string.h>


//Obtiene un numero random del intervalo que recibe por parametros.
int getRandomNumber(int lower, int upper)
{
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}


//Genera una poblacion de individuos con caracteristicas de ataque, velocidad de ataque, y vida.
//Estas se generan aleatorias de 1 a 500.
//Se generan n individuos (la cantidad recibida por parametro)
// Se guarda en una lista de structs population
struct warrior_level *generatePopulation(int individuals)
{
    struct warrior_level *population = malloc(sizeof(struct warrior_level) * individuals);
    int i;
    for(i=0;i<individuals;i++){
        struct warrior_level* individual =(struct warrior_level*)malloc(sizeof(struct warrior_level));
        individual->attack = getRandomNumber(1,500);
        individual->attack_speed = getRandomNumber(1,500);
        individual->health = getRandomNumber(1,500);

        population[i] = *individual;
    }
    return population;
}

// Cruza dos guerreros obteniendo dos caracteristicas aleatorias del otro y guardandolas en el primero.
struct warrior_level getWarriorCrossover(struct warrior_level *warrior1,
                                         struct warrior_level *warrior2){
    struct warrior_level* temp = warrior1;
    char *atributes[3] = {"health","attack","attack_speed"};
    char *atribute_crossover_1 = atributes[getRandomNumber(0,2)];
    char *atribute_crossover_2 = atributes[getRandomNumber(0, 2)];
    while(atribute_crossover_2 == atribute_crossover_1){
        atribute_crossover_2 = atributes[getRandomNumber(0, 2)];
    }
    if(atribute_crossover_1 == "health"){
        warrior2->health = temp->health;
        }
    else if(atribute_crossover_1 == "attack"){
        warrior2->attack = temp->attack;
}
    else if(atribute_crossover_1 == "attack_speed"){
        warrior2->attack_speed = temp->attack_speed;
}

    if(atribute_crossover_2 == "health") {
        warrior2->health = temp->health;
    }
    else if(atribute_crossover_2 == "attack") {
        warrior2->attack = temp->attack;
    }
    else if(atribute_crossover_2 == "attack_speed") {
        warrior2->attack_speed = temp->attack_speed;
    }

    return *warrior2;
}
//Mutate muta una caracteristica seleccionada al azar sumandole 75 o restandole 75 (esto tambien al azar)
void mutate(struct warrior_level *population,int destroyed_warrior1_index) {
    char *atributes[3] = {"health", "attack", "attack_speed"};
    char *atribute_crossover_1 = atributes[getRandomNumber(0, 2)];
    if (atribute_crossover_1 == "health") {
        population[destroyed_warrior1_index].health =
                population[destroyed_warrior1_index].health + (75 * pow(-1, getRandomNumber(1, 2)));
        if(population[destroyed_warrior1_index].health<=0)
            population[destroyed_warrior1_index].health = 1;
        if(population[destroyed_warrior1_index].health>500)
            population[destroyed_warrior1_index].health = 500;
    } else if (atribute_crossover_1 == "attack") {
        population[destroyed_warrior1_index].attack =
                population[destroyed_warrior1_index].attack + (75 * pow(-1, getRandomNumber(1, 2)));
        if(population[destroyed_warrior1_index].attack<=0)
            population[destroyed_warrior1_index].attack = 1;
        if(population[destroyed_warrior1_index].attack>500)
            population[destroyed_warrior1_index].attack = 500;
    } else if (atribute_crossover_1 == "attack_speed") {
        population[destroyed_warrior1_index].attack_speed =
                population[destroyed_warrior1_index].attack_speed + (75 * pow(-1, getRandomNumber(1, 2)));
        if(population[destroyed_warrior1_index].attack_speed<=0)
            population[destroyed_warrior1_index].attack_speed = 1;
        if(population[destroyed_warrior1_index].attack_speed>500)
            population[destroyed_warrior1_index].attack_speed = 500;
    }
}

//llama a la funcion de cruzar y guarda el individuo cruzado en una posicion de la poblacion.
void crossover(struct warrior_level *population,int destroyed_warrior1_index,int destroyed_warrior2_index){
    population[destroyed_warrior1_index] = getWarriorCrossover(&population[destroyed_warrior2_index],&population[destroyed_warrior1_index]);
}



//Imprime una poblacion.
void printPopulation(struct warrior_level *population,int i){
    int n;
    for(n=0;n<i;n++){
        printf(" attack: %d ", population[n].attack);
        printf(" attack_speed: %d ", population[n].attack_speed);
        printf(" health: %d ", population[n].health);
        printf(" level: %d \n", (int) round((float)(population[n].attack+population[n].health+population[n].attack_speed)/1500.0*9));
    }
}

//Toma el struct de un individuo y lo parsea a string.
char *warriorToString (struct warrior_level *warrior, int warrior_number)
{

    size_t len = 0;
    len = snprintf (NULL, len, "Warrior %d Attack: %d A_Speed: %d Health: %d Lvl: %d", warrior_number,warrior->attack,warrior->attack_speed,warrior->health,(int) round((float)(warrior->attack+warrior->health+warrior->attack_speed)/1500.0*9));


    char *apstr = calloc (1, sizeof *apstr * len + 1);


    snprintf (apstr, len+1, "Warrior %d Attack: %d A_Speed: %d Health: %d Lvl: %d", warrior_number,warrior->attack,warrior->attack_speed,warrior->health,(int) round((float)(warrior->attack+warrior->health+warrior->attack_speed)/1500.0*9));

    return apstr;
}


//Obtiene todos los individuos en una lista de strings.
char getCardDeck(struct warrior_level *population, char** choices,int n){
    for(int i = 0; i<n; i++){
        char* warrior_temp = warriorToString(&population[i],i+1);
        choices[i] = warrior_temp;
    }
}


//Realiza la pelea de dos guerreros realizando el dano hacia el sobreviviente.
char* fight(struct warrior_level*warriorA,struct warrior_level *warriorB){
    double dps_warriorA = warriorA->attack*((warriorA->attack_speed/500.0)*1);
    double dps_warriorB = warriorB->attack*((warriorB->attack_speed/500.0)*1);
    if(warriorB->health/dps_warriorA > warriorA->health/dps_warriorB){
        warriorB->health = warriorB->health - ((int) dps_warriorA * warriorA->health/dps_warriorB);
        return "B";
    }
    else if (warriorB->health/dps_warriorA == warriorA->health/dps_warriorB){
        return "tie";
    }
    else{
        warriorA->health = warriorA->health - ((int) dps_warriorB * warriorB->health/dps_warriorA);
        return "A";
    }
}

