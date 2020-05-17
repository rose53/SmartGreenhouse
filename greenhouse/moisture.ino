const int AIR_VALUE   = 874;
const int WATER_VALUE = 466;

const char *description[] = { "saturated", 
                              "adequately wet", 
                              "irrigation advice", 
                              "irrigation", 
                              "dangerously dry"};

/**
 * Get a text vor the moisture
 * @param moisture the relative moisture in percent
 */
const char* getDescription(int moisture) {
    const char* retVal = NULL;
    if (moisture <= 9) {
        retVal = description[0];
    } else if (moisture > 9 && moisture <= 19){
        retVal = description[1];
    } else if (moisture > 19 && moisture <= 59){
        retVal = description[2];
    } else if (moisture > 59 && moisture <= 99){
        retVal = description[3];
    } else {
        retVal = description[4];
    }
    return retVal;
}
                              
/**
 * Calculates the moisture by reading some values and using the median to reduce the possiblity of inaccurate readings 
 */
int getMoisture(int sensor) {

    int moistureArray[5];
    int moistureArrayLenght = sizeof(moistureArray)/sizeof(int);
    // read the values
    for (int i = 0; i < moistureArrayLenght; i++) {
        moistureArray[i] = analogRead(sensor);
        delay(50);
    }
    
    // sort the array, a simple bubblesort is enougth for this amount of samples
    int tmp;
    for (int i = 0; i < moistureArrayLenght - 1; i++) {
        for (int j = i + 1; j < moistureArrayLenght; j++) {
            if (moistureArray[i] > moistureArray[j]) {
                tmp = moistureArray[i];
                moistureArray[i] = moistureArray[j];
                moistureArray[j] = tmp; 
            }
        }
    }
    // the element in the middle of the array is the median
    int value = max(moistureArray[moistureArrayLenght >> 1], WATER_VALUE);
    value = min(value, AIR_VALUE);

    value = (value - WATER_VALUE) * 100 / (AIR_VALUE - WATER_VALUE);
    // return the moisture in [%]
    return value;
}

int getMoistureSesnor1() {
    return getMoisture(A0);
}

int getMoistureSesnor2() {
    return getMoisture(A1);
}
