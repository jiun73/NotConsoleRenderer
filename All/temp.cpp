#################################################################
## Iro
################################################################
##
##* Press Ctrl + '+' / '-' To Zoom in
## * Press Ctrl + S to save and recalculate...
## * Documents are saved to web storage.
## * Only one save slot supported.
## * Matches cannot span lines.
## * Unicode chars must be defined in \u0000 to \uffff format.
## * All matches must be contained by a single group(...)
## * Look behinds not permitted, (? <= or (? < !
    ## * Look forwards are permitted(? = or (? !
        ## * Constants are defined as __my_const = (......)
        ## * The \ = format allows unescaped regular expressions
        ## * Constants referenced by match \ = $${ __my_const }
        ## * Constants can reference other constants
        ## * You are free to delete all the default scopes.
        ## * Twitter : ainslec, Web : http ://eeyo.io/iro
        ##
        ################################################################

        name = mysample
        file_extensions[] = mysample;

################################################################
## Constants
################################################################

__MY_CONSTANT \ = (\b[a - z][a - z0 - 9] *)

################################################################
## Styles
################################################################

styles[]{

.comment : style {
   color = light_green
   italic = true
   ace_scope = comment
   textmate_scope = comment
   pygments_scope = Comment
}

.keyword : style {
   color = cyan
   ace_scope = keyword
   textmate_scope = keyword
   pygments_scope = Keyword
}

.numeric : style {
   color = gold
   ace_scope = constant.numeric
   textmate_scope = constant.numeric
   pygments_scope = Number
}

.punctuation : style {
   color = red_2
   ace_scope = punctuation
   textmate_scope = punctuation
   pygments_scope = Punctuation
}

.text : style {
   color = brown
   ace_scope = text
   textmate_scope = text
   pygments_scope = String
}

.illegal : style {
   color = white
   background_color = red
   ace_scope = invalid
   textmate_scope = invalid
   pygments_scope = Generic.Error
}

}

#################################################
## Parse contexts
#################################################

contexts[]{

##############################################
## Main Context - Entry point context
##############################################

main : context {

   : include "multi_line_comment";

   : pattern {
      regex          \ = (//.*)
      styles[] = .comment;
   }





   : pattern {
      regex          \ = (\bnew\b | \bint\b | \bbool\b | \bdouble\b | \bvector\(\w + \) | \bfalse\b | \btrue\b | \barg\b | \barg\ * \b | \bstring\b | @ | \bfunction\b | \binit\b)
      styles[] = .keyword;
   }

   : pattern {
      regex          \ = (\'\w{1}\')
      styles[] = .text;
   }

   : include "numeric";
   : include "expression";

   : pattern {
      regex          \ = (\B == \B | \s\ | \ | \s | \#\s | \$\s | \bif\b | \bIF\b | \bBUTTON\b | \bTEXTBOX\b | \bTEXT\b)
      styles[] = .punctuation;
   }

   : inline_push {
      regex          \ = (< )
      styles[] = .keyword;
      : pop {
         regex       \ = (> )
         styles[] = .keyword;
      }
      : include "main";
   }



   : inline_push {
      regex          \ = (\")
      styles[] = .text;
      default_style = .text
      : pop {
         regex       \ = (\")
         styles[] = .text;
      }
   }








}

#################################################
## End of Contexts
#################################################

hctype: context
{
   : pattern {
      regex          \ = (\bint\b | \bbool\b | \bdouble\b | \bvector\(\w + \) | \bfalse\b | \btrue\b | \barg\b | \barg\ * \b | \bstring\b | @)
      styles[] = .keyword;
   }
}

expression: context
{
   : inline_push {
      regex          \ = (\()
      styles[] = .keyword;
      default_style = .text
      : pop {
         regex       \ = (\))
         styles[] = .keyword;
      }
   }

   : include "hctype";

   : inline_push {
      regex          \ = (\{)
      styles[] = .keyword;
      : pop {
         regex       \ = (\})
         styles[] = .keyword;
      }
      : include "expression";
   }

   : pattern {
      regex          \ = (\bnew\b | \barg\b | \barg * \b)
      styles[] = .keyword;
   }

   : pattern {
      regex          \ = (: [^ \s; {}<>] + )
      styles[] = .numeric;
   }

   : pattern {
      regex          \ = (\$[^ \s; {}<>] + )
      styles[] = .keyword;
   }
}

###########################################
## Numeric Context
###########################################

numeric: context {
   : pattern {
      regex          \ = (\b\d + )
      styles[] = .numeric;
   }
}

###########################################
## Multi Line Comment Context
###########################################

multi_line_comment: context {
   description = multiline
   : inline_push {
      regex          \ = (/ \*)
      styles[] = .comment;
      default_style = .comment
      : pop {
         regex       \ = (\ * / )
         styles[] = .comment;
      }
   }
}

             }
