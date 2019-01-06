(defun on-enter ()
  (send `(game level goal ,(car args) hide)))
(defun on-leave ()
  ;; TODO(#615): showing goals is not implemented
  (send `(game level goal ,(car args) show)))

