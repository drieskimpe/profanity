/* 
 * preferences.c
 *
 * Copyright (C) 2012 James Booth <boothj5@gmail.com>
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

#include <stdlib.h>
#include <string.h>

#include <glib.h>

static GString *prefs_loc;
static GKeyFile *prefs;

// search logins list
static GSList *logins = NULL;
static GSList *last_found = NULL;
static gchar *search_str = NULL;

static void _save_prefs(void);
static gint _compare_jids(gconstpointer a, gconstpointer b);
static void _reset_login_search(void);
static gchar * _search_logins_from(GSList *curr);

void prefs_load(void)
{
    prefs_loc = g_string_new(getenv("HOME"));
    g_string_append(prefs_loc, "/.profanity");

    prefs = g_key_file_new();
    g_key_file_load_from_file(prefs, prefs_loc->str, G_KEY_FILE_NONE, NULL);

    // create the logins searchable list for autocompletion
    gsize njids;
    gchar **jids =
        g_key_file_get_string_list(prefs, "connections", "logins", &njids, NULL);

    gsize i;
    for (i = 0; i < njids; i++) {
        logins = g_slist_insert_sorted(logins, jids[0], _compare_jids);
    }
}

char * find_login(char *search_str)
{
    gchar *found = NULL;

    if (!logins)
        return NULL;

    if (last_found == NULL) {
        search_str = g_strdup(search_str);
        
        found = _search_logins_from(logins);
        return found;
    } else {
        found = _search_logins_from(g_slist_next(last_found));
        if (found != NULL)
            return found;

        found = _search_logins_from(logins);
        if (found != NULL)
            return found;

        _reset_login_search();
        return NULL;
    }
}

static void _reset_login_search(void)
{
    last_found = NULL;
    if (search_str != NULL)
        free(search_str);
}

static gchar * _search_logins_from(GSList *curr)
{
    while(curr) {
        gchar *curr_jid = curr->data;

        if (strncmp(curr_jid, search_str, strlen(search_str)) == 0) {
            gchar *result = g_strdup(curr_jid);
            last_found = curr;
            
            return result;
        }

        curr = g_slist_next(curr);
    }

    return NULL;
}

static gint _compare_jids(gconstpointer a, gconstpointer b)
{
    const gchar *str_a = (const gchar *) a;
    const gchar *str_b = (const gchar *) b;

    return g_strcmp0(str_a, str_b);
}

gboolean prefs_get_beep(void)
{
    return g_key_file_get_boolean(prefs, "ui", "beep", NULL);
}

void prefs_set_beep(gboolean value)
{
    g_key_file_set_boolean(prefs, "ui", "beep", value);
    _save_prefs();
}

gboolean prefs_get_flash(void)
{
    return g_key_file_get_boolean(prefs, "ui", "flash", NULL);
}

void prefs_set_flash(gboolean value)
{
    g_key_file_set_boolean(prefs, "ui", "flash", value);
    _save_prefs();
}

void prefs_add_login(const char *jid)
{
    gsize njids;
    gchar **jids = 
        g_key_file_get_string_list(prefs, "connections", "logins", &njids, NULL);

    // no logins remembered yet
    if (jids == NULL) {
        njids = 1;
        jids = (gchar**) g_malloc(sizeof(gchar *) * 2);
        jids[0] = g_strdup(jid);
        jids[1] = NULL;
        g_key_file_set_string_list(prefs, "connections", "logins", 
            (const gchar * const *)jids, njids);
        _save_prefs();
        g_strfreev(jids);
        
        return;
    } else {
        gsize i;
        for (i = 0; i < njids; i++) {
            if (strcmp(jid, jids[i]) == 0) {
                g_strfreev(jids);
                return;
            }
        }
    
        // jid not found, add to the list
        jids = (gchar **) g_realloc(jids, (sizeof(gchar *) * (njids+2)));
        jids[njids] = g_strdup(jid);
        njids++;
        jids[njids] = NULL;
        g_key_file_set_string_list(prefs, "connections", "logins",
            (const gchar * const *)jids, njids);
        _save_prefs();
        g_strfreev(jids);

        return;
    }
}

static void _save_prefs(void)
{
    gsize g_data_size;
    char *g_prefs_data = g_key_file_to_data(prefs, &g_data_size, NULL);
    g_file_set_contents(prefs_loc->str, g_prefs_data, g_data_size, NULL);
}
