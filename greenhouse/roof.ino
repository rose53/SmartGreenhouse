
void sendRoofStatus(boolean status) {
    
    if (checkConnection()) {
        StaticJsonDocument<200> doc;

        doc["device"] = "ROOF";
        doc["data"]["status"] = status?"OPEN":"CLOSE";

        serializeJson(doc, buffer);
        client.publish(statusTopic, buffer);
    }    
}

void stopRoof() {
    digitalWrite(MOTOR_PIN1, LOW);
    digitalWrite(MOTOR_PIN2, LOW);
}

void openRoof() {
    digitalWrite(MOTOR_PIN1, HIGH);
    digitalWrite(MOTOR_PIN2, LOW);
    roofOpen = true;
    sendRoofStatus(true);
}


void closeRoof() {
    digitalWrite(MOTOR_PIN1, LOW);
    digitalWrite(MOTOR_PIN2, HIGH);
    roofOpen = false;
    sendRoofStatus(false);
}
