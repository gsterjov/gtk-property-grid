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
