(defmacro while-do(c r . bdy)
   `(do () ((not ,c) ,r) ,@bdy))
