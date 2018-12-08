(defun on-jump ()
  (when (> (get-player-jump-count) 1)
    (hide-label "label_space_to_jump")))
