all: simple_coap_client

CONTIKI = ../../../contiki

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

# linker optimizations
SMALL = 1

# REST Engine shall use Erbium CoAP implementation
APPS += er-coap
APPS += rest-engine

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include

