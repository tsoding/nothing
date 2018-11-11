;;; TODO: script-goal-2.lisp duplicates script-goal-1.lisp
(set on-enter
     (lambda ()
       (hide-goal "goal2")))
(set on-leave
     (lambda ()
       (show-goal "goal2")))
