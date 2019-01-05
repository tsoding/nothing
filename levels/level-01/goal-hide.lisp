(defun on-enter ()
  ;; TODO(#614): hiding goals is not implemented
  (send `(level goal ,(car args) hide)))
(defun on-leave ())

