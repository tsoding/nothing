(defun on-enter ()
  ;; TODO(#614): hiding goals is not implemented
  (send `(game level goal ,(car args) hide)))
(defun on-leave ())

