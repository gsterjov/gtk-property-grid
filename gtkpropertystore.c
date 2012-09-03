/***************************************************************************
 *            gtkpropertystore.c
 *
 *  Jan 17, 2010 1:54:56 PM
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


#include "gtkpropertystore.h"
#include <gtk/gtktreemodel.h>
#include <gobject/gvaluecollector.h>
#include <gtk/gtkprivate.h>
#include <string.h>




/* properties */
enum {
	PROP_0,
	
	PROP_EXPANDER_COLUMN,
	PROP_NAME_COLUMN,
	PROP_TYPE_COLUMN,
	PROP_VALUE_COLUMN
};



struct _GtkPropertyStorePrivate
{
	gint expander_column;
	gint name_column;
	gint type_column;
	gint value_column;
};

typedef struct _GtkPropertyStorePrivate GtkPropertyStorePrivate;


#define GTK_PROPERTY_STORE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GTK_TYPE_PROPERTY_STORE, GtkPropertyStorePrivate))




/* function prototypes */
static void gtk_property_store_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void gtk_tree_model_iface_init (GtkTreeModelIface *iface);


/* interface prototypes */
static void gtk_property_store_get_value (GtkTreeModel *tree_model,
                                          GtkTreeIter  *iter,
                                          gint          column,
                                          GValue       *value);



/* GType */
G_DEFINE_TYPE_WITH_CODE (GtkPropertyStore, gtk_property_store, GTK_TYPE_TREE_STORE,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_MODEL, gtk_tree_model_iface_init))




static void
gtk_tree_model_iface_init (GtkTreeModelIface *iface)
{
	/* virtual interface methods */
	iface->get_value = gtk_property_store_get_value;
}



static void
gtk_property_store_class_init (GtkPropertyStoreClass *class)
{
	GObjectClass *o_class = G_OBJECT_CLASS (class);
	
	g_type_class_add_private (o_class, sizeof (GtkPropertyStorePrivate));
	
	
	/* GObject signals */
	o_class->get_property = gtk_property_store_get_property;
	
	
	/* properties */
	g_object_class_install_property (o_class, PROP_EXPANDER_COLUMN,
	                                 g_param_spec_int ("expander-column",
	                                                   "Expander Column",
	                                                   "The model column for expander states",
	                                                   -1, G_MAXINT, -1,
	                                                   GTK_PARAM_READABLE));
	
	
	g_object_class_install_property (o_class, PROP_NAME_COLUMN,
	                                 g_param_spec_int ("name-column",
	                                                   "Name Column",
	                                                   "The model column for property names",
	                                                   -1, G_MAXINT, -1,
	                                                   GTK_PARAM_READABLE));
	
	
	g_object_class_install_property (o_class, PROP_TYPE_COLUMN,
	                                 g_param_spec_int ("type-column",
	                                                   "Type Column",
	                                                   "The model column for property types",
	                                                   -1, G_MAXINT, -1,
	                                                   GTK_PARAM_READABLE));
	
	
	g_object_class_install_property (o_class, PROP_VALUE_COLUMN,
	                                 g_param_spec_int ("value-column",
	                                                   "Value Column",
	                                                   "The model column for property values",
	                                                   -1, G_MAXINT, -1,
	                                                   GTK_PARAM_READABLE));
}



static void
gtk_property_store_init (GtkPropertyStore *store)
{
	GtkPropertyStorePrivate *priv = GTK_PROPERTY_STORE_GET_PRIVATE (store);
	
	priv->expander_column = -1;
	priv->name_column = -1;
	priv->type_column = -1;
	priv->value_column = -1;
}





static void gtk_property_store_get_value (GtkTreeModel *tree_model,
                                          GtkTreeIter  *iter,
                                          gint          column,
                                          GValue       *value)
{
	GtkPropertyStore *store = GTK_PROPERTY_STORE (tree_model);
	GtkPropertyStorePrivate *priv = GTK_PROPERTY_STORE_GET_PRIVATE (store);
	
	
	if (column == priv->value_column)
	{
		GValue tmp = {0};
		
		GtkTreeModelIface *iface = g_type_interface_peek_parent (GTK_TREE_MODEL_GET_IFACE (tree_model));
		iface->get_value (tree_model, iter, column, &tmp);
		
		
		GValue *val = (GValue*) g_value_peek_pointer (&tmp);
		
		g_value_init (value, G_VALUE_TYPE (val));
		g_value_copy (val, value);
		
		g_value_unset (&tmp);
	}
	
	else
	{
		GtkTreeModelIface *iface = g_type_interface_peek_parent (GTK_TREE_MODEL_GET_IFACE (tree_model));
		iface->get_value (tree_model, iter, column, value);
	}
}





static void
gtk_property_store_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
	GtkPropertyStore        *store = GTK_PROPERTY_STORE (object);
	GtkPropertyStorePrivate *priv  = GTK_PROPERTY_STORE_GET_PRIVATE (store);
	
	
	switch (prop_id)
	{
		case PROP_EXPANDER_COLUMN:
			g_value_set_int (value, gtk_property_store_get_expander_column (store));
			break;
			
		case PROP_NAME_COLUMN:
			g_value_set_int (value, gtk_property_store_get_name_column (store));
			break;
			
		case PROP_TYPE_COLUMN:
			g_value_set_int (value, gtk_property_store_get_type_column (store));
			break;
			
		case PROP_VALUE_COLUMN:
			g_value_set_int (value, gtk_property_store_get_value_column (store));
			break;
			
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}





GtkTreeModel *
gtk_property_store_new (gint n_columns, ...)
{
	GType types[n_columns + 4];
	
	
	if (n_columns > 0)
	{
		gint i;
		va_list args;
		
		va_start (args, n_columns);
		
		for (i = 0; i < n_columns; i++)
			types[i] = va_arg (args, GType);
		
		va_end (args);
	}
	
	
	GtkTreeModel *model = g_object_new (GTK_TYPE_PROPERTY_STORE, NULL);
	GtkPropertyStorePrivate *priv = GTK_PROPERTY_STORE_GET_PRIVATE (model);
	
	
	priv->expander_column = n_columns;
	priv->name_column     = n_columns + 1;
	priv->type_column     = n_columns + 2;
	priv->value_column    = n_columns + 3;
	
	types[n_columns    ] = G_TYPE_BOOLEAN;
	types[n_columns + 1] = G_TYPE_STRING;
	types[n_columns + 2] = G_TYPE_INT;
	types[n_columns + 3] = G_TYPE_VALUE;
	
	
	gtk_tree_store_set_column_types (GTK_TREE_STORE (model), n_columns + 4, types);
	return model;
}




void
gtk_property_store_append_value (GtkPropertyStore *store,
                                 GtkTreeIter      *iter,
                                 GtkTreeIter      *parent,
                                 const gchar      *name,
                                 gint              type,
                                 GValue           *value)
{
	GtkPropertyStorePrivate *priv = GTK_PROPERTY_STORE_GET_PRIVATE (store);
	GtkTreeStore *tree_store = GTK_TREE_STORE (store);
	
	
	if (iter == NULL)
	{
		GtkTreeIter it;
		iter = &it;
	}
	
	
	gtk_tree_store_append (tree_store, iter, parent);
	gtk_tree_store_set    (tree_store, iter,
	                       priv->expander_column, TRUE,
	                       priv->name_column, name,
	                       priv->type_column, type,
	                       priv->value_column, value,
	                       -1);
}



void
gtk_property_store_append (GtkPropertyStore *store,
                           GtkTreeIter      *iter,
                           GtkTreeIter      *parent,
                                             ...)
{
	va_list var_args;
	
	va_start (var_args, parent);
	gtk_property_store_append_valist (store, iter, parent, var_args);
	va_end (var_args);
}



void
gtk_property_store_append_valist (GtkPropertyStore *store,
                                  GtkTreeIter      *iter,
                                  GtkTreeIter      *parent,
                                  va_list           var_args)
{
	const gchar *name = va_arg (var_args, const gchar *);
	
	
	while (name != NULL)
	{
		GType type     = va_arg (var_args, gint);
		GType val_type = va_arg (var_args, GType);
		
		
		if (val_type == G_TYPE_NONE)
		{
			gtk_property_store_append_value (store, iter, parent, name, type, NULL);
			name = va_arg (var_args, const gchar *);
			continue;
		}
		
		
		GValue value = {0};
		gchar *error = NULL;
		
		
		g_value_init (&value, val_type);
		G_VALUE_COLLECT (&value, var_args, 0, &error);
		
		if (error)
		{
			g_warning ("%s: %s", G_STRLOC, error);
			
			g_free (error);
			g_value_unset (&value);
			
			break;
		}
		
		
		gtk_property_store_append_value (store, iter, parent, name, type, &value);
		g_value_unset (&value);
		
		name = va_arg (var_args, const gchar *);
	}
}





void
gtk_property_store_set_value (GtkPropertyStore *store,
                              GtkTreeIter      *iter,
                              gint              column,
                              GValue           *value)
{
	GtkPropertyStorePrivate *priv = GTK_PROPERTY_STORE_GET_PRIVATE (store);
	GtkTreeStore *tree_store = GTK_TREE_STORE (store);
	
	
	/* set the generic value */
	if (column == priv->value_column)
		gtk_tree_store_set (tree_store, iter, column, value, -1);
	
	/* set column value */
	else gtk_tree_store_set_value (tree_store, iter, column, value);
}



void
gtk_property_store_set (GtkPropertyStore *store,
                        GtkTreeIter      *iter,
                                          ...)
{
	va_list var_args;
	
	va_start (var_args, iter);
	gtk_property_store_set_valist (store, iter, var_args);
	va_end (var_args);
}



void
gtk_property_store_set_valist (GtkPropertyStore *store,
                               GtkTreeIter      *iter,
                               va_list           var_args)
{
	gint column = va_arg (var_args, gint);
	
	
	while (column != -1)
	{
		GType val_type = va_arg (var_args, GType);
		
		GValue value = {0};
		gchar *error = NULL;
		
		
		g_value_init (&value, val_type);
		G_VALUE_COLLECT (&value, var_args, 0, &error);
		
		if (error)
		{
			g_warning ("%s: %s", G_STRLOC, error);
			
			g_free (error);
			g_value_unset (&value);
			
			break;
		}
		
		
		gtk_property_store_set_value (store, iter, column, &value);
		g_value_unset (&value);
		
		column = va_arg (var_args, gint);
	}
}





gint
gtk_property_store_get_expander_column (GtkPropertyStore *store)
{
	GtkPropertyStorePrivate *priv = GTK_PROPERTY_STORE_GET_PRIVATE (store);
	return priv->expander_column;
}


gint
gtk_property_store_get_name_column (GtkPropertyStore *store)
{
	GtkPropertyStorePrivate *priv = GTK_PROPERTY_STORE_GET_PRIVATE (store);
	return priv->name_column;
}


gint
gtk_property_store_get_type_column (GtkPropertyStore *store)
{
	GtkPropertyStorePrivate *priv = GTK_PROPERTY_STORE_GET_PRIVATE (store);
	return priv->type_column;
}


gint
gtk_property_store_get_value_column (GtkPropertyStore *store)
{
	GtkPropertyStorePrivate *priv = GTK_PROPERTY_STORE_GET_PRIVATE (store);
	return priv->value_column;
}

