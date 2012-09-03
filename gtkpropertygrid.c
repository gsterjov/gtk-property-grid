/***************************************************************************
 *            gtkpropertygrid.c
 *
 *  Jan 12, 2010 12:26:37 PM
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


#include "gtkpropertygrid.h"
#include <gtk/gtkcelllayout.h>
#include <gtk/gtkprivate.h>



#define IN_RECTANGLE(_x,_y,rect)   ((_y) > (rect)->y && \
                                    (_y) < (rect)->y + (rect)->height && \
                                    (_x) > (rect)->x && \
                                    (_x) < (rect)->x + (rect)->width)



typedef struct _Attribute    Attribute;
typedef struct _RendererInfo RendererInfo;



struct _Attribute
{
	gint column;
	gchar *name;
};


struct _RendererInfo
{
	GtkCellRenderer *renderer;
	
	GList *types;
	GList *attributes;
	
	GtkCellLayoutDataFunc   func_data;
	GtkPropertyGridTypeFunc func_type;
	
	gpointer data_func_data;
	gpointer type_func_data;
};




/* signals */
enum {
	ROW_EXPANDED,
	ROW_COLLAPSED,
	LAST_SIGNAL
};


/* properties */
enum {
	PROP_0,
	
	PROP_MODEL,
	PROP_EXPANDER_COLUMN,
	PROP_NAME_COLUMN,
	PROP_TYPE_COLUMN,
	
	PROP_COLUMN_POSITION,
	PROP_LEVEL_INDENTATION,
	PROP_ENABLE_GRID_LINES,
};



typedef struct _GtkPropertyGridPrivate GtkPropertyGridPrivate;


#define GTK_PROPERTY_GRID_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GTK_TYPE_PROPERTY_GRID, GtkPropertyGridPrivate))

struct _GtkPropertyGridPrivate
{
	GdkWindow *window;
	
	GtkTreeModel *model;
	
	gint name_column;
	gint type_column;
	gint expander_column;
	
	gint column_position;
	gint level_indentation;
	gboolean grid_lines_enabled;
	
	
	GList *renderers;
	
	GtkCellEditable *editable;
	
	
	gboolean mouse_down;
	gboolean resize_column;
	
	
	GtkTreePath  *expander_path;
	GdkRectangle  expander_rect;
	
	GtkTreePath  *selected_path;
	GdkRectangle  selected_rect;
};



/* function prototypes */
static void gtk_property_grid_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void gtk_property_grid_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

/* widget implementation prototypes */
static void gtk_property_grid_realize       (GtkWidget *widget);
static void gtk_property_grid_unrealize     (GtkWidget *widget);
static void gtk_property_grid_size_request  (GtkWidget *widget, GtkRequisition *requisition);
static void gtk_property_grid_size_allocate (GtkWidget *widget, GtkAllocation *allocation);

/* widget event prototypes */
static gboolean gtk_property_grid_expose_event         (GtkWidget *widget, GdkEventExpose *event);
static gboolean gtk_property_grid_motion_notify_event  (GtkWidget *widget, GdkEventMotion *event);
static gboolean gtk_property_grid_button_press_event   (GtkWidget *widget, GdkEventButton *event);
static gboolean gtk_property_grid_button_release_event (GtkWidget *widget, GdkEventButton *event);

/* cell layout implementation prototypes */
static void gtk_property_grid_iface_pack_start (GtkCellLayout *cell_layout, GtkCellRenderer *cell, gboolean expand);
static void gtk_property_grid_iface_pack_end   (GtkCellLayout *cell_layout, GtkCellRenderer *cell, gboolean expand);

static void   gtk_property_grid_iface_clear     (GtkCellLayout *cell_layout);
static GList *gtk_property_grid_iface_get_cells (GtkCellLayout *cell_layout);

static void gtk_property_grid_iface_reorder            (GtkCellLayout *cell_layout, GtkCellRenderer *cell, gint position);
static void gtk_property_grid_iface_set_cell_data_func (GtkCellLayout *cell_layout, GtkCellRenderer *cell, GtkCellLayoutDataFunc func, gpointer func_data, GDestroyNotify destroy);

static void gtk_property_grid_iface_add_attribute    (GtkCellLayout *cell_layout, GtkCellRenderer *cell, const gchar *attribute, gint column);
static void gtk_property_grid_iface_clear_attributes (GtkCellLayout *cell_layout, GtkCellRenderer *cell);


/* interface prototype */
static void gtk_cell_layout_iface_init (GtkCellLayoutIface *iface);


/* signal array */
static guint property_grid_signals [LAST_SIGNAL] = { 0 };



/* GType */
G_DEFINE_TYPE_WITH_CODE (GtkPropertyGrid, gtk_property_grid, GTK_TYPE_CONTAINER,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_CELL_LAYOUT, gtk_cell_layout_iface_init))



static void
marshal_VOID__BOXED_BOXED (GClosure     *closure,
                           GValue       *return_value,
                           guint         n_param_values,
                           const GValue *param_values,
                           gpointer      invocation_hint,
                           gpointer      marshal_data)
{
	typedef void (*GMarshalFunc_VOID__BOXED_BOXED) (gpointer data1,
	                                                gpointer arg_1,
	                                                gpointer arg_2,
	                                                gpointer data2);
	
	register GMarshalFunc_VOID__BOXED_BOXED callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	
	g_return_if_fail (n_param_values == 3);
	
	data1 = g_value_peek_pointer (param_values + 0);
	data2 = closure->data;
	
	callback = (GMarshalFunc_VOID__BOXED_BOXED) (marshal_data ? marshal_data : cc->callback);
	
	callback (data1,
	          g_value_peek_pointer (param_values + 1),
	          g_value_peek_pointer (param_values + 2),
	          data2);
}




static void
gtk_cell_layout_iface_init (GtkCellLayoutIface *iface)
{
	/* virtual interface methods */
	iface->pack_start         = gtk_property_grid_iface_pack_start;
	iface->pack_end           = gtk_property_grid_iface_pack_end;
	iface->clear              = gtk_property_grid_iface_clear;
	iface->get_cells          = gtk_property_grid_iface_get_cells;
	iface->reorder            = gtk_property_grid_iface_reorder;
	iface->set_cell_data_func = gtk_property_grid_iface_set_cell_data_func;
	iface->add_attribute      = gtk_property_grid_iface_add_attribute;
	iface->clear_attributes   = gtk_property_grid_iface_clear_attributes;
}


static void
gtk_property_grid_class_init (GtkPropertyGridClass *class)
{
	GObjectClass      *o_class         = G_OBJECT_CLASS      (class);
	GtkObjectClass    *object_class    = GTK_OBJECT_CLASS    (class);
	GtkWidgetClass    *widget_class    = GTK_WIDGET_CLASS    (class);
	GtkContainerClass *container_class = GTK_CONTAINER_CLASS (class);
	
	g_type_class_add_private (o_class, sizeof (GtkPropertyGridPrivate));
	
	
	/* GObject signals */
	o_class->set_property = gtk_property_grid_set_property;
	o_class->get_property = gtk_property_grid_get_property;
	
	
	/* GtkWidget signals */
	widget_class->realize       = gtk_property_grid_realize;
	widget_class->unrealize     = gtk_property_grid_unrealize;
	widget_class->size_request  = gtk_property_grid_size_request;
	widget_class->size_allocate = gtk_property_grid_size_allocate;
	
	/* GtkWidget event signals */
	widget_class->expose_event         = gtk_property_grid_expose_event;
	widget_class->motion_notify_event  = gtk_property_grid_motion_notify_event;
	widget_class->button_press_event   = gtk_property_grid_button_press_event;
	widget_class->button_release_event = gtk_property_grid_button_release_event;
	
	
	
	/* properties */
	g_object_class_install_property (o_class, PROP_MODEL,
	                                 g_param_spec_object ("model",
	                                                      "Property Grid Model",
	                                                      "The model for the property grid",
	                                                      GTK_TYPE_TREE_MODEL,
	                                                      GTK_PARAM_READWRITE));
	
	
	g_object_class_install_property (o_class, PROP_EXPANDER_COLUMN,
	                                 g_param_spec_int ("expander-column",
	                                                   "Expander Column",
	                                                   "The model column for expander states",
	                                                   -1, G_MAXINT, -1,
	                                                   GTK_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	
	
	g_object_class_install_property (o_class, PROP_NAME_COLUMN,
	                                 g_param_spec_int ("name-column",
	                                                   "Name Column",
	                                                   "The model column for property names",
	                                                   -1, G_MAXINT, -1,
	                                                   GTK_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	
	
	g_object_class_install_property (o_class, PROP_TYPE_COLUMN,
	                                 g_param_spec_int ("type-column",
	                                                   "Type Column",
	                                                   "The model column for property types",
	                                                   -1, G_MAXINT, -1,
	                                                   GTK_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	
	
	g_object_class_install_property (o_class, PROP_COLUMN_POSITION,
	                                 g_param_spec_int ("column-position",
	                                                   "Column Position",
	                                                   "The position for the property grid column",
	                                                   0, G_MAXINT, 100,
	                                                   GTK_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	
	
	g_object_class_install_property (o_class, PROP_LEVEL_INDENTATION,
	                                 g_param_spec_int ("level-indentation",
	                                                   "Level Indentation",
	                                                   "The amount of indentation for child properties",
	                                                   0, G_MAXINT, 10,
	                                                   GTK_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	
	
	g_object_class_install_property (o_class, PROP_ENABLE_GRID_LINES,
	                                 g_param_spec_boolean ("enable-grid-lines",
	                                                       "Enable Grid Lines",
	                                                       "Display property grid lines",
	                                                       TRUE,
	                                                       GTK_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	
	
	
	/* style properties */
	gtk_widget_class_install_style_property (widget_class,
	                                         g_param_spec_int ("expander-size",
	                                                           "Expander Size",
	                                                           "The size of the expander arrow",
	                                                           0, G_MAXINT, 12,
	                                                           GTK_PARAM_READABLE));
	
	
	gtk_widget_class_install_style_property (widget_class,
	                                         g_param_spec_int ("expander-pad",
	                                                           "Expander Pad",
	                                                           "The padding of the expander arrow",
	                                                           0, G_MAXINT, 4,
	                                                           GTK_PARAM_READABLE));
	
	
	gtk_widget_class_install_style_property (widget_class,
	                                         g_param_spec_int ("grid-line-width",
	                                                           "Grid Line Width",
	                                                           "The width of the property grid lines",
	                                                           0, G_MAXINT, 1,
	                                                           GTK_PARAM_READABLE));
	
	
	gtk_widget_class_install_style_property (widget_class,
	                                         g_param_spec_int ("vertical-separator",
	                                                           "Vertical Separator Width",
	                                                           "Vertical space between cells",
	                                                           0, G_MAXINT, 2,
	                                                           GTK_PARAM_READABLE));
	
	
	gtk_widget_class_install_style_property (widget_class,
	                                         g_param_spec_int ("horizontal-separator",
	                                                           "Horizontal Separator Width",
	                                                           "Horizontal space between cells",
	                                                           0, G_MAXINT, 2,
	                                                           GTK_PARAM_READABLE));
	
	
	/* signals */
	property_grid_signals[ROW_EXPANDED] =
			g_signal_new ("row-expanded",
			              G_TYPE_FROM_CLASS (o_class),
			              G_SIGNAL_RUN_LAST,
			              0, NULL, NULL,
			              marshal_VOID__BOXED_BOXED,
			              G_TYPE_NONE, 2,
			              GTK_TYPE_TREE_ITER,
			              GTK_TYPE_TREE_PATH);
	
	
	property_grid_signals[ROW_COLLAPSED] =
			g_signal_new ("row-collapsed",
			              G_TYPE_FROM_CLASS (o_class),
			              G_SIGNAL_RUN_LAST,
			              0, NULL, NULL,
			              marshal_VOID__BOXED_BOXED,
			              G_TYPE_NONE, 2,
			              GTK_TYPE_TREE_ITER,
			              GTK_TYPE_TREE_PATH);
}




static void
gtk_property_grid_init (GtkPropertyGrid *grid)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	/* set flags */
	GTK_WIDGET_SET_FLAGS (grid, GTK_CAN_FOCUS | GTK_RECEIVES_DEFAULT);
	GTK_WIDGET_SET_FLAGS (grid, GTK_NO_WINDOW);
	
	
	priv->model = NULL;
	priv->window = NULL;
	priv->renderers = NULL;
	priv->editable = NULL;
	
	priv->mouse_down = FALSE;
	priv->resize_column = FALSE;
	
	priv->expander_path = NULL;
	priv->selected_path = NULL;
}





void
gtk_property_grid_add_type (GtkPropertyGrid *grid,
                            GtkCellRenderer *cell,
                            gint             type)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	GList *iter;
	
	for (iter = priv->renderers; iter; iter = iter->next)
	{
		RendererInfo *info = (RendererInfo*) iter->data;
		
		if (info->renderer == cell)
			info->types = g_list_append (info->types, GINT_TO_POINTER (type));
	}
}



void
gtk_property_grid_pack_start (GtkPropertyGrid *grid,
                              GtkCellRenderer *cell,
                              gboolean         expand,
                                               ...)
{
	va_list var_args;
	
	va_start (var_args, expand);
	gtk_property_grid_pack_valist (grid, cell, expand, var_args);
	va_end (var_args);
}



void
gtk_property_grid_pack_valist (GtkPropertyGrid *grid,
                               GtkCellRenderer *cell,
                               gboolean         expand,
                               va_list          var_args)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	/* create renderer info */
	RendererInfo *info = g_slice_new0 (RendererInfo);
	info->renderer = cell;
	info->types = NULL;
	
	/* add to renderer list */
	priv->renderers = g_list_append (priv->renderers, info);
	
	
	gint type = va_arg (var_args, gint);
	
	while (type != -1)
	{
		info->types = g_list_append (info->types, GINT_TO_POINTER (type));
		type = va_arg (var_args, gint);
	}
}



static void
gtk_property_grid_iface_pack_start (GtkCellLayout   *cell_layout,
                                    GtkCellRenderer *cell,
                                    gboolean         expand)
{
	GtkPropertyGrid *grid = GTK_PROPERTY_GRID (cell_layout);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	gtk_property_grid_pack_start (grid, cell, expand, -1);
}



static void
gtk_property_grid_iface_pack_end (GtkCellLayout   *cell_layout,
                                  GtkCellRenderer *cell,
                                  gboolean         expand)
{
	GtkPropertyGrid *grid = GTK_PROPERTY_GRID (cell_layout);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
}



static void
gtk_property_grid_iface_clear (GtkCellLayout *cell_layout)
{
	GtkPropertyGrid *grid = GTK_PROPERTY_GRID (cell_layout);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	GList *iter;
	
	/* free allocated renderer info */
	for (iter = priv->renderers; iter; iter = iter->next)
		g_slice_free (RendererInfo, iter->data);
	
	
	g_list_free (priv->renderers);
	priv->renderers = NULL;
}



static GList *
gtk_property_grid_iface_get_cells (GtkCellLayout *cell_layout)
{
	GtkPropertyGrid *grid = GTK_PROPERTY_GRID (cell_layout);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
}



static void
gtk_property_grid_iface_reorder (GtkCellLayout   *cell_layout,
                                 GtkCellRenderer *cell,
                                 gint             position)
{
	GtkPropertyGrid *grid = GTK_PROPERTY_GRID (cell_layout);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
}



static void
gtk_property_grid_iface_add_attribute (GtkCellLayout   *cell_layout,
                                       GtkCellRenderer *cell,
                                       const gchar     *attribute,
                                       gint             column)
{
	GtkPropertyGrid *grid = GTK_PROPERTY_GRID (cell_layout);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	GList *iter;
	
	/* search for matching cell renderer */
	for (iter = priv->renderers; iter; iter = iter->next)
	{
		RendererInfo *info = (RendererInfo*) iter->data;
		
		/* found cell renderer */
		if (info->renderer == cell)
		{
			/* create attribute */
			Attribute *attr = g_slice_new (Attribute);
			attr->column = column;
			attr->name = g_strdup (attribute);
			
			/* add attribute to the list */
			info->attributes = g_list_append (info->attributes, attr);
		}
	}
}



static void
gtk_property_grid_iface_clear_attributes (GtkCellLayout   *cell_layout,
                                          GtkCellRenderer *cell)
{
	GtkPropertyGrid *grid = GTK_PROPERTY_GRID (cell_layout);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	GList *iter;
	
	/* search for matching cell renderer */
	for (iter = priv->renderers; iter; iter = iter->next)
	{
		RendererInfo *info = (RendererInfo*) iter->data;
		
		/* found cell renderer */
		if (info->renderer == cell)
		{
			GList *it;
			
			/* free all attributes */
			for (it = info->attributes; it; it = it->next)
			{
				Attribute *attr = (Attribute*) it->data;
				
				g_free (attr->name);
				g_slice_free (Attribute, attr);
			}
			
			g_list_free (info->attributes);
		}
	}
}



static void
gtk_property_grid_iface_set_cell_data_func (GtkCellLayout         *cell_layout,
                                            GtkCellRenderer       *cell,
                                            GtkCellLayoutDataFunc  func,
                                            gpointer               func_data,
                                            GDestroyNotify         destroy)
{
	GtkPropertyGrid *grid = GTK_PROPERTY_GRID (cell_layout);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	GList *iter;
	
	/* search for matching cell renderer */
	for (iter = priv->renderers; iter; iter = iter->next)
	{
		RendererInfo *info = (RendererInfo*) iter->data;
		
		/* found cell renderer */
		if (info->renderer == cell)
		{
			info->func_data = func;
			info->data_func_data = func_data;
		}
	}
}



void
gtk_property_grid_set_cell_type_func (GtkPropertyGrid         *cell_layout,
                                      GtkCellRenderer         *cell,
                                      GtkPropertyGridTypeFunc  func,
                                      gpointer                 func_data)
{
	GtkPropertyGrid *grid = GTK_PROPERTY_GRID (cell_layout);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	GList *iter;
	
	/* search for matching cell renderer */
	for (iter = priv->renderers; iter; iter = iter->next)
	{
		RendererInfo *info = (RendererInfo*) iter->data;
		
		/* found cell renderer */
		if (info->renderer == cell)
		{
			info->func_type = func;
			info->type_func_data = func_data;
		}
	}
}






static void
gtk_property_grid_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
	GtkPropertyGrid        *grid = GTK_PROPERTY_GRID (object);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	
	switch (prop_id)
	{
		case PROP_MODEL:
			gtk_property_grid_set_model (grid, GTK_TREE_MODEL (g_value_get_object (value)));
			break;
			
		case PROP_EXPANDER_COLUMN:
			gtk_property_grid_set_expander_column (grid, g_value_get_int (value));
			break;
			
		case PROP_NAME_COLUMN:
			gtk_property_grid_set_name_column (grid, g_value_get_int (value));
			break;
			
		case PROP_TYPE_COLUMN:
			gtk_property_grid_set_type_column (grid, g_value_get_int (value));
			break;
			
			
		case PROP_COLUMN_POSITION:
			gtk_property_grid_set_column_position (grid, g_value_get_int (value));
			break;
			
		case PROP_LEVEL_INDENTATION:
			gtk_property_grid_set_level_indentation (grid, g_value_get_int (value));
			break;
			
		case PROP_ENABLE_GRID_LINES:
			gtk_property_grid_set_enable_grid_lines (grid, g_value_get_boolean (value));
			break;
			
			
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}




static void
gtk_property_grid_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
	GtkPropertyGrid        *grid = GTK_PROPERTY_GRID (object);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	
	switch (prop_id)
	{
		case PROP_MODEL:
			g_value_set_object (value, G_OBJECT (gtk_property_grid_get_model (grid)));
			break;
			
		case PROP_EXPANDER_COLUMN:
			g_value_set_int (value, gtk_property_grid_get_expander_column (grid));
			break;
			
		case PROP_NAME_COLUMN:
			g_value_set_int (value, gtk_property_grid_get_name_column (grid));
			break;
			
		case PROP_TYPE_COLUMN:
			g_value_set_int (value, gtk_property_grid_get_type_column (grid));
			break;
			
			
		case PROP_COLUMN_POSITION:
			g_value_set_int (value, gtk_property_grid_get_column_position (grid));
			break;
			
		case PROP_LEVEL_INDENTATION:
			g_value_set_int (value, gtk_property_grid_get_level_indentation (grid));
			break;
			
		case PROP_ENABLE_GRID_LINES:
			g_value_set_boolean (value, gtk_property_grid_get_enable_grid_lines (grid));
			break;
			
			
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}




static void
on_remove_editable (GtkCellEditable *cell_editable, gpointer user_data)
{
	GtkPropertyGrid *grid = GTK_PROPERTY_GRID (user_data);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	
	/* remove editable widget */
	if (priv->editable == cell_editable)
	{
		priv->editable = NULL;
		gtk_widget_queue_resize (GTK_WIDGET (grid));
	}
	
	
	/* destroy editable */
	GtkWidget *widget = GTK_WIDGET (cell_editable);
	
	gtk_widget_unparent (widget);
	gtk_widget_destroy (widget);
}



static gboolean
get_next_property (GtkPropertyGrid *grid, GtkTreeIter *iter)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	GtkTreeIter next;
	gboolean expanded = FALSE;
	gboolean got_next = FALSE;
	
	
	/* got expander column */
	if (priv->expander_column >= 0)
		gtk_tree_model_get (priv->model, iter, priv->expander_column, &expanded, -1);
	
	
	/* property has children and is expanded */
	if (expanded && gtk_tree_model_iter_has_child (priv->model, iter))
		got_next = gtk_tree_model_iter_children (priv->model, &next, iter);
	
	
	/* go to next sibling property */
	else
	{
		next = *iter;
		
		/* get the next property */
		got_next = gtk_tree_model_iter_next (priv->model, &next);
		
		/* reached the end of the node list so go up a level */
		if (!got_next)
		{
			if (gtk_tree_model_iter_parent (priv->model, &next, iter))
				got_next = gtk_tree_model_iter_next (priv->model, &next);
		}
	}
	
	
	/* set next property */
	*iter = next;
	return got_next;
}




static gboolean
can_render_value (GtkPropertyGrid *grid, GtkTreeIter *iter, RendererInfo *info)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	/* get renderer type */
	if (info->func_type)
		return info->func_type (grid, priv->model, iter, info->type_func_data);
	
	
	
	gint type = -1;
	
	/* get property type */
	if (priv->type_column >= 0)
		gtk_tree_model_get (priv->model, iter, priv->type_column, &type, -1);
	
	
	GList *it;
	
	/* try to match a registered type */
	for (it = info->types; it; it = it->next)
	{
		/* found a match. can render data type */
		if (type == GPOINTER_TO_INT (it->data))
			return TRUE;
	}
	
	
	/* cannot render */
	return FALSE;
}



static void
set_property_value (GtkPropertyGrid *grid, GtkTreeIter *iter, RendererInfo *info)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	/* set property data */
	if (info->func_data)
		info->func_data (GTK_CELL_LAYOUT (grid),
		                 info->renderer,
		                 priv->model,
		                 iter,
		                 info->data_func_data);
	
	
	/* use attributes to set property data */
	else
	{
		GList *attr_it;
		
		/* set all properties from renderer attributes */
		for (attr_it = info->attributes; attr_it; attr_it = attr_it->next)
		{
			Attribute *attr = (Attribute*) attr_it->data;
			
			/* get generic value */
			GValue value = {0};
			gtk_tree_model_get_value (priv->model, iter, attr->column, &value);
			
			/* set renderer property by value */
			g_object_set_property (G_OBJECT (info->renderer), attr->name, &value);
			
			
			/* clean up */
			g_value_unset (&value);
		}
	}
}




static void
get_row_size (GtkPropertyGrid *grid, GtkTreeIter *iter, int *width, int *height)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	GtkWidget *widget = GTK_WIDGET (grid);
	
	const gchar *name = NULL;
	gint ypad;
	
	/* get style properties */
	gtk_widget_style_get (widget, "horizontal-separator", &ypad, NULL);
	
	
	/* get property name */
	if (priv->name_column >= 0)
		gtk_tree_model_get (priv->model, iter, priv->name_column, &name, -1);
	
	
	
	/* get max property name size */
	PangoLayout *layout = gtk_widget_create_pango_layout (widget, name);
	pango_layout_get_pixel_size (layout, NULL, height);
	
	/* free layout */
	g_object_unref (layout);
	
	
	/* apply padding */
	*width = 0;
	*height += ypad * 2;
	
	
	/* property has children */
	if (gtk_tree_model_iter_has_child (priv->model, iter))
	{
		gint expander_size;
		
		/* get style properties */
		gtk_widget_style_get (widget, "expander-size", &expander_size, NULL);
		
		/* apply padding */
		expander_size += ypad * 2;
		
		/* expander is bigger than property name height */
		*height = MAX (expander_size, *height);
	}
	
	
	/* leaf property */
	else
	{
		GList *it = NULL;
		
		/* find max property value size */
		for (it = priv->renderers; it; it = it->next)
		{
			RendererInfo *info = (RendererInfo*) it->data;
			
			/* cell renderer cant render this property */
			if (!can_render_value (grid, iter, info))
				continue;
			
			
			/* set cell value */
			set_property_value (grid, iter, info);
			
			
			/* get cell size */
			gint x, y, w, h;
			gtk_cell_renderer_get_size (info->renderer, widget, NULL, &x, &y, &w, &h);
			
			/* add offsets */
			w += x;
			h += y;
			
			/* set max size */
			*width  = MAX (w, *width);
			*height = MAX (h, *height);
		}
		
		
		/* get minimum editable size */
		if (priv->editable)
		{
			GtkTreePath *path = gtk_tree_model_get_path (priv->model, iter);
			
			/* property is in editing mode */
			if (priv->selected_path && gtk_tree_path_compare (path, priv->selected_path) == 0)
			{
				GtkRequisition requisition;
				gtk_widget_size_request (GTK_WIDGET (priv->editable), &requisition);
				
				*width  = MAX (requisition.width,  *width);
				*height = MAX (requisition.height, *height);
			}
			
			/* free path */
			gtk_tree_path_free (path);
		}
		
	}
}




static gboolean
get_cell_at_pos (GtkPropertyGrid *grid,
                 gdouble x, gdouble y,
                 GdkRectangle *rect,
                 GtkTreeIter  *iter)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	GtkWidget *widget = GTK_WIDGET (grid);
	
	
	gint top = 0;
	gint line_width;
	
	gtk_widget_style_get (widget, "grid-line-width", &line_width, NULL);
	
	
	/* get first property */
	gboolean got_next = gtk_tree_model_get_iter_first (priv->model, iter);
	
	
	/* loop through all properties */
	while (got_next)
	{
		/* get row size */
		gint w, h;
		get_row_size (grid, iter, &w, &h);
		
		/* set size */
		rect->x = 0;
		rect->y = top;
		rect->height = h;
		
		
		/* property has children */
		if (gtk_tree_model_iter_has_child (priv->model, iter))
		{
			GtkAllocation alloc;
			gtk_widget_get_allocation (widget, &alloc);
			
			rect->width = alloc.width;
			
			/* cell is a parent property */
			if (IN_RECTANGLE (x, y, rect))
				return TRUE;
		}
		
		
		/* leaf property */
		else
		{
			/* set property name dimensions */
			rect->width = priv->column_position;
			
			/* cell is a property name */
			if (IN_RECTANGLE (x, y, rect))
				return TRUE;
			
			
			GtkAllocation alloc;
			gtk_widget_get_allocation (widget, &alloc);
			
			/* set property value dimensions */
			rect->x = priv->column_position + line_width;
			rect->width = alloc.width - rect->x;
			
			/* cell is a property value */
			if (IN_RECTANGLE (x, y, rect))
				return TRUE;
		}
		
		
		/* go to next cell */
		top += h + line_width;
		
		/* get the next property */
		got_next = get_next_property (grid, iter);
	}
	
	
	return FALSE;
}




static gboolean
get_row_at_iter (GtkPropertyGrid *grid, GtkTreeIter *iter, GdkRectangle *rect)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	GtkWidget *widget = GTK_WIDGET (grid);
	
	gint top = 0;
	gint line_width;
	
	/* get style properties */
	gtk_widget_style_get (widget, "grid-line-width", &line_width, NULL);
	
	
	
	/* get first property */
	GtkTreeIter it;
	gboolean got_next = gtk_tree_model_get_iter_first (priv->model, &it);
	
	GtkTreePath *iter_path = gtk_tree_model_get_path (priv->model, iter);
	
	
	/* loop through all properties */
	while (got_next)
	{
		/* get row size */
		gint w, h;
		get_row_size (grid, &it, &w, &h);
		
		
		/* convert iterator to paths */
		GtkTreePath *path = gtk_tree_model_get_path (priv->model, &it);
		
		/* found property */
		if (gtk_tree_path_compare (path, iter_path) == 0)
		{
			GtkAllocation alloc;
			gtk_widget_get_allocation (widget, &alloc);
			
			/* set size */
			rect->x = 0;
			rect->y = top;
			rect->width = alloc.width;
			rect->height = h;
			
			
			/* clean up */
			gtk_tree_path_free (path);
			gtk_tree_path_free (iter_path);
			
			return TRUE;
		}
		
		
		/* clean up */
		gtk_tree_path_free (path);
		
		
		/* go to the next cell */
		top += h + line_width;
		
		/* get the next property */
		got_next = get_next_property (grid, &it);
	}
	
	
	/* clean up */
	gtk_tree_path_free (iter_path);
	
	return FALSE;
}




static void
clear_selection (GtkPropertyGrid *grid)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	/* clear last selected area */
	if (priv->selected_path)
	{
		gtk_tree_path_free (priv->selected_path);
		priv->selected_path = NULL;
		
		gdk_window_invalidate_rect (priv->window, &priv->selected_rect, FALSE);
	}
	
	/* remove last editable widget */
	if (priv->editable)
		gtk_cell_editable_remove_widget (priv->editable);
}





static void
paint_property_name (GtkPropertyGrid *grid,
                     GtkTreeIter     *iter,
                     GdkRectangle    *expose_area,
                     GdkRectangle    *area)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	GtkWidget *widget = GTK_WIDGET (grid);
	
	
	gint expander_size, expander_pad;
	gint xpad, ypad;
	
	/* get style properties */
	gtk_widget_style_get (widget,
	                      "expander-size", &expander_size,
	                      "expander-pad", &expander_pad,
	                      "vertical-separator", &xpad,
	                      "horizontal-separator", &ypad,
	                      NULL);
	
	
	const gchar *name = NULL;
	
	/* get property name */
	if (priv->name_column >= 0)
		gtk_tree_model_get (priv->model, iter, priv->name_column, &name, -1);
	
	
	/* get property state */
	GtkTreePath *path = gtk_tree_model_get_path (priv->model, iter);
	GtkStateType state = GTK_STATE_NORMAL;
	
	
	/* property is selected */
	if (priv->selected_path && gtk_tree_path_compare (path, priv->selected_path) == 0)
		state = GTK_STATE_SELECTED;
	
	
	GtkStyle *style = gtk_widget_get_style (widget);
	
	
	/* paint background */
	gtk_paint_flat_box (style,
	                    priv->window,
	                    state,
	                    GTK_SHADOW_NONE,
	                    expose_area,
	                    widget,
	                    "",
	                    0, area->y,
	                    area->width + area->x, area->height);
	
	
	/* property has children */
	if (gtk_tree_model_iter_has_child (priv->model, iter))
	{
		gboolean expanded = FALSE;
		GtkStateType expander_state = GTK_STATE_NORMAL;
		
		/* get expander state */
		if (priv->expander_path && gtk_tree_path_compare (path, priv->expander_path) == 0)
			expander_state = GTK_STATE_PRELIGHT;
		
		
		/* got expander column */
		if (priv->expander_column >= 0)
			gtk_tree_model_get (priv->model, iter, priv->expander_column, &expanded, -1);
		
		
		/* paint expander */
		gtk_paint_expander (style,
		                    priv->window,
		                    expander_state,
		                    expose_area,
		                    widget,
		                    "gridview",
		                    area->x + xpad + (expander_size / 2),
		                    area->y + ypad + (expander_size / 2),
		                    expanded ? GTK_EXPANDER_EXPANDED : GTK_EXPANDER_COLLAPSED);
	}
	
	
	/* create pango layout */
	PangoLayout *layout = gtk_widget_create_pango_layout (widget, name);
	
	
	/* clamp layout width */
	gint space = area->width - expander_size - expander_pad - (xpad * 2);
	pango_layout_set_width (layout, space * PANGO_SCALE);
	
	
	/* create layout clip region */
	GdkRectangle clip;
	GdkRectangle rect;
	
	rect.x = area->x;
	rect.y = area->y;
	rect.width  = area->width - xpad;
	rect.height = area->height - ypad;
	
	gdk_rectangle_intersect (&rect, expose_area, &clip);
	
	
	/* paint property name */
	gtk_paint_layout (style,
	                  priv->window,
	                  state,
	                  TRUE,
	                  &clip,
	                  widget,
	                  "label_fg",
	                  area->x + xpad + expander_size + expander_pad,
	                  area->y + ypad,
	                  layout);
	
	
	/* clean up */
	gtk_tree_path_free (path);
	g_object_unref (layout);
}




static void
paint_property_value (GtkPropertyGrid *grid,
                      GtkTreeIter     *iter,
                      GdkRectangle    *expose_area,
                      GdkRectangle    *area)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	GtkWidget *widget = GTK_WIDGET (grid);
	
	
	gint type = 0;
	
	/* get property type */
	if (priv->type_column >= 0)
		gtk_tree_model_get (priv->model, iter, priv->type_column, &type, -1);
	
	
	GList *it;
	
	/* render all cells */
	for (it = priv->renderers; it; it = it->next)
	{
		RendererInfo *info = (RendererInfo*) it->data;
		
		/* cell renderer cant render this property */
		if (!can_render_value (grid, iter, info))
			continue;
		
		
		/* set cell data */
		set_property_value (grid, iter, info);
		
		
		/* get cell size */
		gint x, y, w, h;
		gtk_cell_renderer_get_size (info->renderer, widget, NULL, &x, &y, &w, &h);
		
		/* create property value rectangle */
		GdkRectangle rect;
		rect.x = area->x + x;
		rect.y = area->y + y;
		rect.width = area->width;
		rect.height = area->height;
		
		
		/* render the cell */
		gtk_cell_renderer_render (info->renderer, priv->window, widget, area, &rect, expose_area, 0);
	}
}





static gint
paint_property (GtkPropertyGrid *grid,
                GtkTreeIter     *iter,
                GdkRectangle    *expose_area,
                GdkRectangle    *area,
                cairo_t         *cr)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	GtkWidget *widget = GTK_WIDGET (grid);
	
	
	gint line_width;
	
	/* get style properties */
	gtk_widget_style_get (widget, "grid-line-width", &line_width, NULL);
	
	
	/* total property height including child properties and padding */
	gint total_height = 0;
	
	
	/* get row size */
	gint width, height;
	get_row_size (grid, iter, &width, &height);
	
	
	/* create cell rectangle */
	GdkRectangle rect;
	rect.x = area->x;
	rect.y = area->y;
	rect.width = area->width;
	rect.height = height;
	
	
	/* paint horizontal grid line */
	if (priv->grid_lines_enabled && area->y > 0)
	{
		GtkAllocation alloc;
		gtk_widget_get_allocation (widget, &alloc);
		
		gdouble pos = area->y + (line_width / 2);
		
		cairo_move_to (cr, 0, pos);
		cairo_line_to (cr, alloc.width, pos);
		
		rect.y += line_width;
	}
	
	
	
	/* property has children */
	if (gtk_tree_model_iter_has_child (priv->model, iter))
	{
		gboolean expanded = FALSE;
		
		/* got expander column */
		if (priv->expander_column >= 0)
			gtk_tree_model_get (priv->model, iter, priv->expander_column, &expanded, -1);
		
		
		/* paint parent property */
		paint_property_name (grid, iter, expose_area, &rect);
		
		
		/* paint child properties */
		if (expanded)
		{
			gint expander_size, indent;
			
			/* get style properties */
			gtk_widget_style_get (widget, "expander-size", &expander_size, NULL);
			g_object_get (widget, "level-indentation", &indent, NULL);
			
			
			/* apply indentation */
			rect.x += indent;
			rect.y += rect.height;
			rect.width -= rect.x;
			
			
			/* get first child */
			GtkTreeIter it;
			gboolean got_next = gtk_tree_model_iter_children (priv->model, &it, iter);
			
			
			/* paint children under this property */
			while (got_next)
			{
				gint h = paint_property (grid, &it, expose_area, &rect, cr);
				
				rect.y += h;
				total_height += h;
				
				got_next = gtk_tree_model_iter_next (priv->model, &it);
			}
		}
	}
	
	
	/* leaf property */
	else
	{
		/* clamp name width to column position */
		rect.width = priv->column_position - rect.x;
		
		/* paint property name */
		paint_property_name (grid, iter, expose_area, &rect);
		
		
		GtkAllocation alloc;
		gtk_widget_get_allocation (widget, &alloc);
		
		/* clamp value width to available space after column position */
		rect.x = priv->column_position + line_width;
		rect.width = alloc.width - rect.x;
		
		paint_property_value (grid, iter, expose_area, &rect);
	}
	
	
	/* add this property to the final height */
	total_height += rect.height + line_width;
	
	return total_height;
}




static void
gtk_property_grid_realize (GtkWidget *widget)
{
	GtkPropertyGrid        *grid = GTK_PROPERTY_GRID (widget);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	GdkWindowAttr attributes;
	GtkAllocation allocation;
	
	
	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
	
	
	/* get current allocation */
	gtk_widget_get_allocation (widget, &allocation);
	
	
	/* set gdk window attributes */
	attributes.x      = allocation.x;
	attributes.y      = allocation.y;
	attributes.width  = allocation.width;
	attributes.height = allocation.height;
	
	attributes.event_mask = GDK_EXPOSURE_MASK |
	                        GDK_BUTTON_PRESS_MASK |
	                        GDK_BUTTON_RELEASE_MASK |
	                        GDK_POINTER_MOTION_MASK |
	                        gtk_widget_get_events (widget);
	
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.wclass = GDK_INPUT_OUTPUT;
	
	
	/* create window */
	priv->window = gdk_window_new (gtk_widget_get_parent_window (widget),
	                               &attributes,
	                               GDK_WA_X | GDK_WA_Y);
	
	gtk_widget_set_window (widget, priv->window);
	gdk_window_set_user_data (priv->window, widget);
	
	
	GTK_WIDGET_UNSET_FLAGS (widget, GTK_NO_WINDOW);
}




static void
gtk_property_grid_unrealize (GtkWidget *widget)
{
	GtkPropertyGrid        *grid = GTK_PROPERTY_GRID (widget);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	gdk_window_clear (priv->window);
	GTK_WIDGET_CLASS (gtk_property_grid_parent_class)->unrealize (widget);
}




static void
gtk_property_grid_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	GtkPropertyGrid        *grid = GTK_PROPERTY_GRID (widget);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	
	gint line_width;
	
	/* get style properties */
	gtk_widget_style_get (widget, "grid-line-width", &line_width, NULL);
	
	
	/* default requisition */
	requisition->width = 0;
	requisition->height = 0;
	
	
	/* add minimum size of properties */
	if (priv->model)
	{
		gint count = 0;
		GtkTreeIter iter;
		
		gboolean got_next = gtk_tree_model_get_iter_first (priv->model, &iter);
		
		
		/* get minimum size of all property cells */
		while (got_next)
		{
			/* get row size */
			gint width, height;
			get_row_size (grid, &iter, &width, &height);
			
			
			/* get largest minimum width */
			requisition->width = MAX (width, requisition->width);
			
			/* add the minimum row height */
			requisition->height += height;
			
			
			/* get the next property */
			got_next = get_next_property (grid, &iter);
		}
		
		
		/* add grid lines to overall height */
		requisition->height += line_width * (count - 1);
		count++;
	}
	
	
	/* add column position to minimum width */
	requisition->width += priv->column_position + line_width;
}




static void
gtk_property_grid_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GtkPropertyGrid        *grid = GTK_PROPERTY_GRID (widget);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	
	/* set allocation */
	gtk_widget_set_allocation (widget, allocation);
	
	
	/* resize editable widget */
	if (priv->editable)
	{
		gint line_width;
		GtkAllocation rect;
		
		/* get style properties */
		gtk_widget_style_get (widget, "grid-line-width", &line_width, NULL);
		
		
		/* get new editable allocation */
		rect.x = priv->selected_rect.x + priv->column_position + line_width;
		rect.y = priv->selected_rect.y;
		rect.width = allocation->width - rect.x;
		rect.height = priv->selected_rect.height;
		
		/* resize editable */
		gtk_widget_size_allocate (GTK_WIDGET (priv->editable), &rect);
	}
	
	
	/* resize gdk window */
	if (priv->window)
		gdk_window_move_resize (priv->window,
		                        allocation->x,
		                        allocation->y,
		                        allocation->width,
		                        allocation->height);
}




static gboolean
gtk_property_grid_expose_event (GtkWidget *widget, GdkEventExpose *event)
{
	GtkPropertyGrid        *grid = GTK_PROPERTY_GRID (widget);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	
	gint line_width;
	GtkStyle *style;
	
	
	/* get style properties */
	gtk_widget_style_get (widget, "grid-line-width", &line_width, NULL);
	style = gtk_widget_get_style (widget);
	
	
	/* get cairo context */
	cairo_t *cr = gdk_cairo_create (GDK_DRAWABLE (priv->window));
	
	
	/* set line properties */
	cairo_set_line_width (cr, line_width);
	cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
	
	/* set line colour */
	gdk_cairo_set_source_color (cr, &style->fg[GTK_STATE_NORMAL]);
	
	
	/* get exposed area */
	GdkRectangle *expose_area = &event->area;
	GtkAllocation area;
	gtk_widget_get_allocation (widget, &area);
	
	
	/* clip drawing area */
	gdk_cairo_rectangle (cr, expose_area);
	cairo_clip (cr);
	
	
	
	/* default widget background */
	gtk_style_apply_default_background (style,
	                                    priv->window,
	                                    TRUE,
	                                    GTK_STATE_NORMAL,
	                                    expose_area,
	                                    0, 0,
	                                    priv->column_position,
	                                    area.height);
	
	
	/* paint property value column background */
	gtk_paint_flat_box (style,
	                    priv->window,
	                    GTK_STATE_NORMAL,
	                    GTK_SHADOW_NONE,
	                    expose_area,
	                    widget,
	                    "entry_bg",
	                    priv->column_position, 0,
	                    area.width - priv->column_position,
	                    area.height);
	
	
	/* vertical grid line */
	if (priv->grid_lines_enabled)
	{
		gdouble pos = priv->column_position + (line_width / 2);
		
		cairo_move_to (cr, pos, 0);
		cairo_line_to (cr, pos, area.height);
		
		/* stroke now so parent properties paint
		 * on top of the vertical line */
		cairo_stroke (cr);
	}
	
	
	
	/* paint all properties */
	if (priv->model)
	{
		/* property rectangle */
		GdkRectangle rect;
		rect.x = area.x;
		rect.y = area.y;
		rect.width = area.width;
		rect.height = 0;
		
		
		/* get first property */
		GtkTreeIter iter;
		gboolean got_next = gtk_tree_model_get_iter_first (priv->model, &iter);
		
		while (got_next)
		{
			/* paint property and get its final height */
			rect.y += paint_property (grid, &iter, expose_area, &rect, cr);
			got_next = gtk_tree_model_iter_next (priv->model, &iter);
		}
		
		
		/* paint horizontal grid lines */
		cairo_stroke (cr);
	}
	
	
	/* clean up */
	cairo_destroy (cr);
	
	return TRUE;
}




static gboolean
gtk_property_grid_motion_notify_event (GtkWidget *widget, GdkEventMotion *event)
{
	GtkPropertyGrid        *grid = GTK_PROPERTY_GRID (widget);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	
	/* mouse dragging */
	if (priv->mouse_down & priv->resize_column)
	{
		GtkAllocation alloc;
		gtk_widget_get_allocation (widget, &alloc);
		
		/* ensure column position is in the valid range */
		priv->column_position = MAX (event->x, 0);
		priv->column_position = MIN (priv->column_position, alloc.width);
		
		
		/* calculate minimum size */
		gtk_widget_queue_resize (widget);
		
		/* invalidate entire widget */
		gtk_widget_queue_draw (widget);
		
		
		return TRUE;
	}
	
	
	gint line_width;
	
	/* get style properties */
	gtk_widget_style_get (widget, "grid-line-width", &line_width, NULL);
	
	
	/* pointer in column rectangle */
	if (event->x > (priv->column_position - 2) &&
	    event->x < (priv->column_position + line_width + 2))
	{
		if (!priv->resize_column)
		{
			/* set resize cursor */
			GdkCursor *cursor = gdk_cursor_new (GDK_SB_H_DOUBLE_ARROW);
			gdk_window_set_cursor (priv->window, cursor);
			gdk_cursor_unref (cursor);
			
			priv->resize_column = TRUE;
		}
		
		return TRUE;
	}
	
	
	/* reset cursor */
	else if (priv->resize_column)
	{
		gdk_window_set_cursor (priv->window, NULL);
		priv->resize_column = FALSE;
	}
	
	
	/* no model set */
	if (!priv->model) return TRUE;
	
	GdkRectangle rect;
	GtkTreeIter iter;
	
	
	/* found cell at pointer position */
	if (get_cell_at_pos (grid, event->x, event->y, &rect, &iter))
	{
		/* parent property */
		if (gtk_tree_model_iter_has_child (priv->model, &iter))
		{
			gint expander_size, xpad, ypad;
			
			/* get style properties */
			gtk_widget_style_get (widget,
			                      "expander-size", &expander_size,
			                      "vertical-separator", &xpad,
			                      "horizontal-separator", &ypad,
			                      NULL);
			
			
			GtkTreePath *path = gtk_tree_model_get_path (priv->model, &iter);
			gint depth = gtk_tree_path_get_depth (path) - 1;
			
			/* set the expander rectangle. we include x and y padding
			 * to allow for 'fuzzy' selection */
			rect.x = priv->level_indentation * depth;
			rect.width = expander_size + (xpad * 2);
			rect.height = expander_size + (ypad * 2);
			
			
			/* mouse pointer is in the expander rectangle */
			if (IN_RECTANGLE (event->x, event->y, &rect))
			{
				if (!priv->expander_path)
				{
					priv->expander_path = path;
					priv->expander_rect = rect;
					
					gdk_window_invalidate_rect (priv->window, &rect, FALSE);
				}
				
				/* clean up */
				else if (gtk_tree_path_compare (path, priv->expander_path) != 0)
					gtk_tree_path_free (path);
				
				
				return TRUE;
			}
			
			
			/* clean up */
			gtk_tree_path_free (path);
		}
	}
	
	
	/* mouse pointer not over an expander */
	if (priv->expander_path)
	{
		gtk_tree_path_free (priv->expander_path);
		priv->expander_path = NULL;
		
		gdk_window_invalidate_rect (priv->window, &priv->expander_rect, FALSE);
	}
	
	
	return TRUE;
}




static gboolean
gtk_property_grid_button_press_event (GtkWidget *widget, GdkEventButton *event)
{
	GtkPropertyGrid        *grid = GTK_PROPERTY_GRID (widget);
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	
	
	/* left mouse button pressed */
	if (!event->button == 1)
		return TRUE;
	
	
	priv->mouse_down = TRUE;
	gtk_widget_grab_focus (widget);
	
	
	/* resizing the column or no model set */
	if (priv->resize_column || !priv->model)
		return TRUE;
	
	
	/* remove last selection and editable */
	clear_selection (grid);
	
	
	/* clicked on an expander */
	if (priv->expander_path && IN_RECTANGLE (event->x, event->y, &priv->expander_rect))
	{
		/* got expander column */
		if (priv->expander_column >= 0)
		{
			gboolean expanded;
			GtkTreeIter iter;
			
			/* expand or contract the property */
			if (gtk_tree_model_get_iter (priv->model, &iter, priv->expander_path))
			{
				gtk_tree_model_get (priv->model, &iter, priv->expander_column, &expanded, -1);
				
				/* call expanded signals */
				if (expanded)
					g_signal_emit_by_name (grid, "row-collapsed", &iter, priv->expander_path);
				else
					g_signal_emit_by_name (grid, "row-expanded", &iter, priv->expander_path);
				
				
				/* calculate minimum size */
				gtk_widget_queue_resize (widget);
			}
		}
		
		return TRUE;
	}
	
	
	
	/* cell properties */
	GdkRectangle rect;
	GtkTreeIter iter;
	
	
	/* found a cell that was clicked on */
	if (get_cell_at_pos (grid, event->x, event->y, &rect, &iter))
	{
		GtkTreePath *path = gtk_tree_model_get_path (priv->model, &iter);
		
		
		/* value cell clicked */
		if (rect.x > priv->column_position)
		{
			GList *it = NULL;
			
			/* activate appropriate cell renderers */
			for (it = priv->renderers; it; it = it->next)
			{
				RendererInfo *info = (RendererInfo*) it->data;
				
				/* cell renderer cant render this property */
				if (!can_render_value (grid, &iter, info))
					continue;
				
				
				/* get cell renderer mode */
				GtkCellRendererMode mode;
				g_object_get (info->renderer, "mode", &mode, NULL);
				
				
				/* property is activatable */
				if (mode == GTK_CELL_RENDERER_MODE_ACTIVATABLE)
				{
					/* activate selected cell */
					gtk_cell_renderer_activate (info->renderer,
					                            (GdkEvent*) event,
					                            widget,
					                            gtk_tree_path_to_string (path),
					                            &rect,
					                            &rect,
					                            0);
					
					
					/* draw cell renderer */
					gdk_window_invalidate_rect (priv->window, &rect, FALSE);
				}
				
				
				/* property is editable */
				else if (mode == GTK_CELL_RENDERER_MODE_EDITABLE)
				{
					gint line_width;
					
					/* get style properties */
					gtk_widget_style_get (widget, "grid-line-width", &line_width, NULL);
					
					
					GtkAllocation alloc;
					gtk_widget_get_allocation (widget, &alloc);
					
					/* get actual cell coords */
					get_row_at_iter (grid, &iter, &rect);
					
					rect.x = priv->column_position + line_width;
					rect.width = alloc.width - rect.x;
					
					
					priv->editable = gtk_cell_renderer_start_editing (info->renderer,
					                                                  (GdkEvent*) event,
					                                                  widget,
					                                                  gtk_tree_path_to_string (path),
					                                                  &rect,
					                                                  &rect,
					                                                  0);
					
					/* go into edit mode */
					if (priv->editable)
					{
						GtkWidget *wid = GTK_WIDGET (priv->editable);
						
						/* get minimum editable size */
						GtkRequisition requisition;
						gtk_widget_size_request (wid, &requisition);
						
						rect.width  = MAX (requisition.width,  rect.width);
						rect.height = MAX (requisition.height, rect.height);
						
						/* show editable widget */
						gtk_widget_set_parent (wid, widget);
						gtk_widget_size_allocate (wid, &rect);
						gtk_widget_show_all (wid);
						
						gtk_cell_editable_start_editing (priv->editable, (GdkEvent*) event);
						
						gtk_widget_grab_focus (wid);
						
						/* connect to remove-widget signal */
						g_signal_connect (priv->editable,
						                  "remove-widget",
						                  G_CALLBACK (on_remove_editable),
						                  grid);
						
						
						/* calculate minimum size */
						gtk_widget_queue_resize (widget);
					}
				}
				
			} /* end cell renderer loop */
		} /* end value cell */
		
		
		
		/* set new selection */
		priv->selected_path = path;
		
		priv->selected_rect.x = 0;
		priv->selected_rect.y = rect.y;
		priv->selected_rect.width = priv->column_position;
		priv->selected_rect.height = rect.height;
		
		/* parent properties take up the whole width */
		if (gtk_tree_model_iter_has_child (priv->model, &iter))
		{
			GtkAllocation alloc;
			gtk_widget_get_allocation (widget, &alloc);
			
			priv->selected_rect.width = alloc.width;
		}
		
		
		/* draw new selection */
		gdk_window_invalidate_rect (priv->window, &priv->selected_rect, FALSE);
	}
}




static gboolean
gtk_property_grid_button_release_event (GtkWidget *widget, GdkEventButton *event)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (widget);
	
	priv->mouse_down = FALSE;
	return TRUE;
}





GtkWidget *
gtk_property_grid_new (void)
{
	return g_object_new (GTK_TYPE_PROPERTY_GRID, NULL);
}




void
gtk_property_grid_set_model (GtkPropertyGrid *grid, GtkTreeModel *model)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	priv->model = model;
}


GtkTreeModel *
gtk_property_grid_get_model (GtkPropertyGrid *grid)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	return priv->model;
}



void
gtk_property_grid_set_expander_column (GtkPropertyGrid *grid, gint column)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	priv->expander_column = column;
}


gint
gtk_property_grid_get_expander_column (GtkPropertyGrid *grid)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	return priv->expander_column;
}



void
gtk_property_grid_set_name_column (GtkPropertyGrid *grid, gint column)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	priv->name_column = column;
}


gint
gtk_property_grid_get_name_column (GtkPropertyGrid *grid)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	return priv->name_column;
}



void
gtk_property_grid_set_type_column (GtkPropertyGrid *grid, gint column)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	priv->type_column = column;
}


gint
gtk_property_grid_get_type_column (GtkPropertyGrid *grid)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	return priv->type_column;
}




void
gtk_property_grid_set_column_position (GtkPropertyGrid *grid, gint position)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	priv->column_position = position;
}


gint
gtk_property_grid_get_column_position (GtkPropertyGrid *grid)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	return priv->column_position;
}



void
gtk_property_grid_set_level_indentation (GtkPropertyGrid *grid, gint indentation)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	priv->level_indentation = indentation;
}


gint
gtk_property_grid_get_level_indentation (GtkPropertyGrid *grid)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	return priv->level_indentation;
}



void
gtk_property_grid_set_enable_grid_lines (GtkPropertyGrid *grid, gboolean enabled)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	priv->grid_lines_enabled = enabled;
}


gboolean
gtk_property_grid_get_enable_grid_lines (GtkPropertyGrid *grid)
{
	GtkPropertyGridPrivate *priv = GTK_PROPERTY_GRID_GET_PRIVATE (grid);
	return priv->grid_lines_enabled;
}



