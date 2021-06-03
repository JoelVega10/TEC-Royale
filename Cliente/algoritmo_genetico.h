//
// Created by joel on 8/5/21.
//

#ifndef ALGORITMO_GENETICO_AG_H
#define ALGORITMO_GENETICO_AG_H

typedef struct warrior_level{
    int health;
    int attack;
    int attack_speed;
    int level;
}warrior_level;

int getRandomNumber(int lower, int upper);
struct warrior_level *generatePopulation(int individuals);
struct warrior_level getWarriorCrossover(struct warrior_level *warrior1,struct warrior_level *warrior2);
void mutate(struct warrior_level *population,int destroyed_warrior1_index);
void printPopulation(struct warrior_level *population,int i);
char *warriorToString (struct warrior_level *warrior, int warrior_number);
void crossover(struct warrior_level *population,int destroyed_warrior1_index,int destroyed_warrior2_index);
char getCardDeck(struct warrior_level *population, char** choices,int n);
char* fight(struct warrior_level *warriorA,struct warrior_level *warriorB);


#endif //ALGORITMO_GENETICO_AG_H
