### Projectos que iremos compilar, por ex:
CONTIKI_PROJECT = sensores_e_botoes leds_aleatorios cooja_z1_temp exercicio_3
### Eventuais drivers de sensores que necessitamos
### (tipicamente contidos em ~/contiki/platform/zoul/dev):
#CONTIKI_TARGET_SOURCEFILES += dht22.c tsl256x.c
### Quando fizermos "make", iremos compilar todos os projectos mencionados acima:
all: $(CONTIKI_PROJECT)

# Linker optimizations
SMALL = 1

# Includes the project-conf configuration file
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

### Localização da pasta root "contiki" em relação à pasta actual:
CONTIKI = ../

# This flag includes the IPv6 libraries
CONTIKI_WITH_IPV6 = 1

include $(CONTIKI)/Makefile.include