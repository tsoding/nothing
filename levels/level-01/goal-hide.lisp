(defun on-enter ()
  ;; TODO: hiding goals is not implemented
  (send `(level goal ,(car args) hide)))
(defun on-leave ())

