/*
 * macros.h
 *
 * Copyright (C) 2002 Rob Caelers <robc@krandor.org>
 * All rights reserved.
 *
 * Time-stamp: <2002-11-13 21:08:02 robc>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * $Id$
 *
 */

#ifndef MACROS_H
#define MACROS_H

#define WR_METHOD(rettype, method, args...) \
	static rettype static_##method (PortableServer_Servant, args, CORBA_Environment *); \
        rettype method ( args )

#define WR_METHOD_NOARGS(rettype, method) \
	static rettype static_##method (PortableServer_Servant, CORBA_Environment *); \
        rettype method ( void )


#define WR_METHOD_ARGS0_IMPL(rettype, method) \
	rettype WR_C_CLASS::static_##method (PortableServer_Servant s, CORBA_Environment *ev)    \
        {                                                                                               \
                WR_CLASS *obj = WR_CAST(bonobo_object_from_servant(s));                           \
                WR_C_CLASS *c = obj->_this;                                                             \
                c->method ();                                                                     \
        }                                                                                               \
        rettype WR_C_CLASS::method ( )

#define WR_METHOD_ARGS1_IMPL(rettype, method, t1, v1) \
	rettype WR_C_CLASS::static_##method (PortableServer_Servant s, t1 v1, CORBA_Environment *ev)    \
        {                                                                                               \
                WR_CLASS *obj = WR_CAST(bonobo_object_from_servant(s));                           \
                WR_C_CLASS *c = obj->_this;                                                             \
                c->method ( v1 );                                                                     \
        }                                                                                               \
        rettype WR_C_CLASS::method ( t1 v1 )

#define WR_METHOD_ARGS2_IMPL(rettype, method, t1, v1, t2, v2) \
	rettype WR_C_CLASS::static_##method (PortableServer_Servant s,  t1 v1, t2 v2, CORBA_Environment *ev)    \
        {                                                                                               \
                WR_CLASS *obj = WR_CAST(bonobo_object_from_servant(s));                           \
                WR_C_CLASS *c = obj->_this;                                                             \
                c->method ( v1, v2 );                                                                     \
        }                                                                                               \
        rettype WR_C_CLASS::method ( t1 v1, t2 v2 )

#define WR_METHOD_ARGS3_IMPL(rettype, method, t1, v1, t2, v2, t3, v3) \
	rettype WR_C_CLASS::static_##method (PortableServer_Servant s, t1 v1, t2 v2, t3 v3 , \
                                             CORBA_Environment *ev)    \
        {                                                                                               \
                WR_CLASS *obj = WR_CAST(bonobo_object_from_servant(s));                           \
                WR_C_CLASS *c = obj->_this;                                                             \
                c->method ( v1, v2, v3 );                                                               \
        }                                                                                               \
        rettype WR_C_CLASS::method ( t1 v1, t2 v2, t3 v3 )

#define WR_METHOD_ARGS4_IMPL(rettype, method, t1, v1, t2, v2, t3, v3, t4, v4) \
	rettype WR_C_CLASS::static_##method (PortableServer_Servant s, t1 v1, t2 v2, t3 v3, t4, v4, \
                                             CORBA_Environment *ev)    \
        {                                                                                               \
                WR_CLASS *obj = WR_CAST(bonobo_object_from_servant(s));                           \
                WR_C_CLASS *c = obj->_this;                                                             \
                c->method ( v1, v2, v3, v4 );                                                           \
        }                                                                                               \
        rettype WR_C_CLASS::method ( t1 v1, t2 v2, t3 v3, t4, v4 )

#define WR_REG_METHOD1(x,y) epv -> y = &WR_C_CLASS :: x ## _ ## y ;
#define WR_REG_METHOD2(x,y) WR_REG_METHOD1(x,y)
#define WR_REG_METHOD3(x) WR_REG_METHOD2(static, x)

#define WR_INIT1(x,y) \
        static void x ## _class_init(y ## Class *) ;                                        \
        static void x ## _init(y *) ;                                        		\
        static GType x ## _get_type() ;                                        		\
        static WR_CLASS * x ## _new(void) ;                                        \
         
#define WR_INIT2(x,y) WR_INIT1(x,y)
#define WR_INIT3() WR_INIT2(WR_PREFIX, WR_CLASS)


#define WR_METHOD_REGISTER(x) WR_REG_METHOD3(x)
#define WR_INIT() WR_INIT3()
     
#endif /* MACROS_H */
