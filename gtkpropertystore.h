/***************************************************************************
 *            gtkpropertystore.h
 *
 *  Jan 17, 2010 1:54:38 PM
 *  Copyright  2010  Goran Sterjov
 *  <goran.sterjov@gmail.com>
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with main.c; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#ifndef __GTK_PROPERTY_STORE_H__
#define __GTK_PROPERTY_STORE_H__


#include <gtk/gtktreestore.h>


G_BEGIN_DECLS

#define GTK_TYPE_PROPERTY_STORE                 (gtk_property_store_get_type ())
#define GTK_PROPERTY_STORE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_PROPERTY_STORE, GtkPropertyStore))
#define GTK_PROPERTY_STORE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_PROPERTY_STORE, GtkPropertyStoreClass))
#define GTK_IS_PROPERTY_STORE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_PROPERTY_STORE))
#define GTK_IS_PROPERTY_STORE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PROPERTY_STORE))
#define GTK_PROPERTY_STORE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_PROPERTY_STORE, GtkPropertyStoreClass))


typedef struct _GtkPropertyStore      GtkPropertyStore;
typedef struct _GtkPropertyStoreClass GtkPropertyStoreClass;


struct _GtkPropertyStore
{
	GtkTreeStore parent;
};


struct _GtkPropertyStoreClass
{
	GtkTreeStoreClass parent_class;
};



GType         gtk_property_store_get_type (void) G_GNUC_CONST;
GtkTreeModel *gtk_property_store_new      (gint columns, ...);



void gtk_property_store_append_value  (GtkPropertyStore *store,
                                       GtkTreeIter      *iter,
                                       GtkTreeIter      *parent,
                                       const gchar      *name,
                                       gint              type,
                                       GValue           *value);

void gtk_property_store_append        (GtkPropertyStore *store,
                                       GtkTreeIter      *iter,
                                       GtkTreeIter      *parent,
                                                         ...);

void gtk_property_store_append_valist (GtkPropertyStore *store,
                                       GtkTreeIter      *iter,
                                       GtkTreeIter      *parent,
                                       va_list           var_args);



void gtk_property_store_set_value  (GtkPropertyStore *store,
                                    GtkTreeIter      *iter,
                                    gint              column,
                                    GValue           *value);

void gtk_property_store_set        (GtkPropertyStore *store,
                                    GtkTreeIter      *iter,
                                                      ...);

void gtk_property_store_set_valist (GtkPropertyStore *store,
                                    GtkTreeIter      *iter,
                                    va_list           var_args);



gint gtk_property_store_get_expander_column (GtkPropertyStore *store);
gint gtk_property_store_get_name_column     (GtkPropertyStore *store);
gint gtk_property_store_get_type_column     (GtkPropertyStore *store);
gint gtk_property_store_get_value_column    (GtkPropertyStore *store);



G_END_DECLS

#endif /* __GTK_PROPERTY_STORE_H__ */
