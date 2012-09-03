/***************************************************************************
 *            gtkpropertygrid.h
 *
 *  Jan 12, 2010 12:02:43 PM
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
 * Foundation, Inc., 51 Franklin Sgridt, Fifth Floor Boston, MA 02110-1301,  USA
 */

#ifndef __GTK_PROPERTY_GRID_H__
#define __GTK_PROPERTY_GRID_H__


#include <gtk/gtkcontainer.h>
#include <gtk/gtktreemodel.h>
#include <gtk/gtkcellrenderer.h>


G_BEGIN_DECLS

#define GTK_TYPE_PROPERTY_GRID                 (gtk_property_grid_get_type ())
#define GTK_PROPERTY_GRID(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_PROPERTY_GRID, GtkPropertyGrid))
#define GTK_PROPERTY_GRID_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_PROPERTY_GRID, GtkPropertyGridClass))
#define GTK_IS_PROPERTY_GRID(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_PROPERTY_GRID))
#define GTK_IS_PROPERTY_GRID_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PROPERTY_GRID))
#define GTK_PROPERTY_GRID_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_PROPERTY_GRID, GtkPropertyGridClass))


typedef struct _GtkPropertyGrid      GtkPropertyGrid;
typedef struct _GtkPropertyGridClass GtkPropertyGridClass;


typedef gboolean (*GtkPropertyGridTypeFunc) (GtkPropertyGrid *grid,
                                             GtkTreeModel    *model,
                                             GtkTreeIter     *iter,
                                             gpointer         data);


struct _GtkPropertyGrid
{
	GtkContainer parent;
};


struct _GtkPropertyGridClass
{
	GtkContainerClass parent_class;
};



GType      gtk_property_grid_get_type (void) G_GNUC_CONST;
GtkWidget *gtk_property_grid_new      (void);



void          gtk_property_grid_set_model (GtkPropertyGrid *grid, GtkTreeModel *model);
GtkTreeModel *gtk_property_grid_get_model (GtkPropertyGrid *grid);



void gtk_property_grid_add_type (GtkPropertyGrid *grid,
                                 GtkCellRenderer *cell,
                                 gint             type);


void gtk_property_grid_pack_start (GtkPropertyGrid *grid,
                                   GtkCellRenderer *cell,
                                   gboolean         expand,
                                                    ...);


void gtk_property_grid_pack_valist (GtkPropertyGrid *grid,
                                    GtkCellRenderer *cell,
                                    gboolean         expand,
                                    va_list          var_args);


void gtk_property_grid_set_cell_type_func (GtkPropertyGrid         *grid,
                                           GtkCellRenderer         *cell,
                                           GtkPropertyGridTypeFunc  func,
                                           gpointer                 func_data);



void gtk_property_grid_set_expander_column (GtkPropertyGrid *grid, gint column);
gint gtk_property_grid_get_expander_column (GtkPropertyGrid *grid);

void gtk_property_grid_set_name_column (GtkPropertyGrid *grid, gint column);
gint gtk_property_grid_get_name_column (GtkPropertyGrid *grid);

void gtk_property_grid_set_type_column (GtkPropertyGrid *grid, gint column);
gint gtk_property_grid_get_type_column (GtkPropertyGrid *grid);


void gtk_property_grid_set_column_position (GtkPropertyGrid *grid, gint position);
gint gtk_property_grid_get_column_position (GtkPropertyGrid *grid);

void gtk_property_grid_set_level_indentation (GtkPropertyGrid *grid, gint indentation);
gint gtk_property_grid_get_level_indentation (GtkPropertyGrid *grid);

void     gtk_property_grid_set_enable_grid_lines (GtkPropertyGrid *grid, gboolean enabled);
gboolean gtk_property_grid_get_enable_grid_lines (GtkPropertyGrid *grid);



G_END_DECLS

#endif /* __GTK_PROPERTY_GRID_H__ */
