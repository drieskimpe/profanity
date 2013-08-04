/*
 * api.c
 *
 * Copyright (C) 2012, 2013 James Booth <boothj5@gmail.com>
 *
 * This file is part of Profanity.
 *
 * Profanity is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Profanity is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Profanity.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <Python.h>

#include "plugins/command.h"
#include "ui/notifier.h"
#include "ui/ui.h"

static PyObject*
api_cons_show(PyObject *self, PyObject *args)
{
    const char *message = NULL;

    if (!PyArg_ParseTuple(args, "s", &message)) {
        return NULL;
    }
    cons_show("%s", message);

    return Py_BuildValue("");
}

static PyObject*
api_register_command(PyObject *self, PyObject *args)
{
    const char *command_name = NULL;
    int min_args = 0;
    int max_args = 0;
    const char *usage = NULL;
    const char *short_help = NULL;
    const char *long_help = NULL;
    PyObject *p_callback = NULL;

    if (!PyArg_ParseTuple(args, "siisssO", &command_name, &min_args, &max_args, &usage, &short_help, &long_help, &p_callback)) {
        return NULL;
    }

    if (p_callback && PyCallable_Check(p_callback)) {
        PluginCommand *command = malloc(sizeof(PluginCommand));
        command->command_name = command_name;
        command->min_args = min_args;
        command->max_args = max_args;
        command->usage = usage;
        command->short_help = short_help;
        command->long_help = long_help;
        command->p_callback = p_callback;

        add_command(command);
    }

    return Py_BuildValue("");
}

static PyObject *
api_register_timed(PyObject *self, PyObject *args)
{
    PyObject *p_callback = NULL;
    int interval_ms = 0;

    if (!PyArg_ParseTuple(args, "Oi", &p_callback, &interval_ms)) {
        return NULL;
    }

    return Py_BuildValue("");
}

static PyObject*
api_notify(PyObject *self, PyObject *args)
{
    const char *message = NULL;
    const char *category = NULL;
    int timeout_ms = 5000;

    if (!PyArg_ParseTuple(args, "sis", &message, &timeout_ms, &category)) {
        return NULL;
    }

    notify(message, timeout_ms, category);

    return Py_BuildValue("");
}

static PyMethodDef apiMethods[] = {
    { "cons_show", api_cons_show, METH_VARARGS, "Print a line to the console." },
    { "register_command", api_register_command, METH_VARARGS, "Register a command." },
    { "register_timed", api_register_timed, METH_VARARGS, "Register a timed function." },
    { "notify", api_notify, METH_VARARGS, "Send desktop notification." },
    { NULL, NULL, 0, NULL }
};

void
api_init(void)
{
    Py_InitModule("prof", apiMethods);
}
