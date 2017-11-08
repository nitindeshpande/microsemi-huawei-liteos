/*!
    \file  netconf.h
    \brief the header file of netconf 
*/

/*
    Copyright (C) 2016 GigaDevice

    2016-10-19, V1.0.0, demo for GD32F4xx
*/

#ifndef NETCONF_H
#define NETCONF_H

/* function declarations */
/* initializes the LwIP stack */
void lwip_stack_init(void);
/* dhcp_task */
void dhcp_task(void * pvParameters);

#endif /* NETCONF_H */
