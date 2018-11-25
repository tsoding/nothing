(set on-enter
     (lambda ()
       ;; TODO(#526): ebisp doesn't have `when`
       ;; TODO(#527): ebisp doesn't have `>`
       (when (> (get-player-jump-count) 1)
         (hide-goal "label_space_to_jump"))))
(set on-leave
     (lambda ()))
