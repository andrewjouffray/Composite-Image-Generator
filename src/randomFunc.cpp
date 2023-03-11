#include "../include/randomFunc.h"

using namespace std;

float randomFloat(float from, float to) {
        random_device                    rand_dev;
        mt19937                          generator(rand_dev());
        uniform_real_distribution<float>    distr(from, to);
        return distr(generator);
}

int randomInt(int from, int to) {
        random_device                    rand_dev;
        mt19937                          generator(rand_dev());
        uniform_int_distribution<int>    distr(from, to);
        return distr(generator);
}

