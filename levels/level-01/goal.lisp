(defun on-enter ()
  (send `(game level goal ,(car args) hide)))
(defun on-leave ()
  (send `(game level goal ,(car args) show)))

