#ifndef BUZZER_H
#define BUZZER_H

void buzzer_init(void);
void buzzer_alert(void);      // Alarme contínuo
void buzzer_confirm(void);    // Bipe curto de confirmação
void buzzer_stop(void);

#endif
