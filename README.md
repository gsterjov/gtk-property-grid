gtk-property-grid
=================

GTK+2 property grid with a generic property store

The GTK+2 tree view requires a homogeneous data type for each column, however
for applications that required a list of properties with different types it
cannot use the traditional MVC structure of the treeview.

This project adds a new GTK+2 widget called the PropertyGrid which renders
its cell according to the data type in the associated store. However, stores
too need a homogeneous data type for each column. Included with the PropertyGrid
is a PropertyStore that allows generic values within a column associated with
a data type column so the PropertyGrid knows how to render the specified type.

As a result the PropertyGrid can render anything from a colour cell to an image
browser cell.

Basic property grid usage.
--------------------------

The code below shows the basic process for connecting a tree model with the property grid as well as populating the generic property store


```c
		/* create property store */
		GtkTreeStore *store = gtk_property_store_new (0);
		gint expander_column, name_column, type_column, value_column;
		
		g_object_get (store,
		              "expander-column", &expander_column,
		              "name-column", &name_column,
		              "type-column", &type_column,
		              "value-column", &value_column,
		              NULL);
		
		
		/* setup property tree */
		GtkWidget *grid = gtk_property_grid_new ();
		
		
		g_object_set (grid
		              "expander-column", expander_column,
		              "name-column", name_column,
		              "type-column", type_column,
		              NULL);
		
		gtk_property_grid_set_model (GTK_PROPERTY_GRID (grid), store);		
		g_signal_connect (grid, "row-collapsed", G_CALLBACK (row_collapsed), NULL);
		g_signal_connect (grid, "row-expanded", G_CALLBACK (row_expanded), NULL);
		
		
		
		GdkColor col = {0, 0, 0, 0};
		
		GtkTreeIter it;
		gtk_property_store_append (GTK_PROPERTY_STORE (store), &it, NULL, "Basic Properties", PROPERTY_NONE, G_TYPE_NONE, NULL);
		
		gtk_property_store_append (GTK_PROPERTY_STORE (store), NULL, &it,
		                           "Ambient", PROPERTY_AMBIENT, GDK_TYPE_COLOR, &col,
		                           "Diffuse", PROPERTY_DIFFUSE, GDK_TYPE_COLOR, &col,
		                           NULL);
		
		GtkTreeIter spec;
		gtk_property_store_append (GTK_PROPERTY_STORE (store), &spec, &it, "Specular", PROPERTY_NONE, G_TYPE_NONE, NULL);
		
		
		gtk_property_store_append (GTK_PROPERTY_STORE (store), NULL, &it,
		                           "Emissive", PROPERTY_EMISSIVE, GDK_TYPE_COLOR, &col,
		                           "Lighting", PROPERTY_LIGHTING, G_TYPE_BOOLEAN, TRUE,
		                           "Shading", PROPERTY_SHADING, GTK_TYPE_LIST_STORE, shading_store,
		                           NULL);
		
		
		gtk_property_store_append (GTK_PROPERTY_STORE (store), NULL, &spec,
		                           "Colour", PROPERTY_SPECULAR, GDK_TYPE_COLOR, &col,
		                           "Shininess", PROPERTY_SHININESS, G_TYPE_INT, 1,
		                           NULL);
		
		
		
		gtk_property_grid_pack_start (GTK_PROPERTY_GRID (grid),
		                              GTK_CELL_RENDERER (colour_renderer),
		                              TRUE,
		                              PROPERTY_AMBIENT,
		                              PROPERTY_DIFFUSE,
		                              PROPERTY_SPECULAR,
		                              PROPERTY_EMISSIVE,
		                              -1);
		
		gtk_property_grid_pack_start (GTK_PROPERTY_GRID (grid),
		                              GTK_CELL_RENDERER (toggle_renderer),
		                              TRUE,
		                              PROPERTY_LIGHTING,
		                              -1);
		
		gtk_property_grid_pack_start (GTK_PROPERTY_GRID (grid),
		                              GTK_CELL_RENDERER (spin_renderer),
		                              TRUE,
		                              PROPERTY_SHININESS,
		                              -1);
		
		gtk_property_grid_pack_start (GTK_PROPERTY_GRID (grid),
		                              GTK_CELL_RENDERER (combo_renderer),
		                              TRUE,
		                              PROPERTY_SHADING,
		                              -1);
		
		
		
		gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (grid), GTK_CELL_RENDERER (colour_renderer), "colour", value_column);
		gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (grid), GTK_CELL_RENDERER (toggle_renderer), "active", value_column);
		gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (grid), GTK_CELL_RENDERER (spin_renderer), "text", value_column);
		gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (grid), GTK_CELL_RENDERER (combo_renderer), "model", value_column);
```
