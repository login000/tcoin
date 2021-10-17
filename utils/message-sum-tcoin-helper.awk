#!/usr/bin/gawk -Mf

#mst.awk -> message_sum_tcoin.awk

BEGIN {
  supersummary=0
  summary=0
  sum=0
  sumplus=0
  summinus=0
  self_user=""
  self_user_found=0
  FS=""
}

(NR == 1) {
  #Determining onlyo (only other-user) from first line (see /home/login/mst)
  if($0 ~ /^[a-zA-Z0-9_.\-]+/) {
    if(substr($0,NF-12) == "_messages.txt") {
      onlyo=substr($0,1,NF-13)
    }
    else {
      onlyo=$0
    }
  }
}

(NR != 1) {
  term=0
  other_user=""
  other_user_found=0
  if ($0 == "" || $1==" ") {
    next
  }

  for (a=1;a!=NF;a++) {
    # other-user determination
    if ($a == ":" && a == 25) {
      for (b=a+1;;b++) {
        if ($b ~ /^[a-zA-Z0-9_\-]/) {
          for(c=b;;c++) {
            if ($c ~ /^[^a-zA-Z0-9_\-]?$/) {
              break
            }
            else {
              other_user = ((other_user) ($c))
            }
          }
          break
        }
      }
      other_user_found=1
    }
    # self-username determination
    else if ($a == "<" && !self_user_found) {
      for (b=a+1;;b++) {
        if ($b == " ") {
          for(c=b+1;;c++) {
            if($c ~ /^[^a-zA-Z0-9_\-]?$/) {
              break
            }
            else {
              self_user = ((self_user) ($c))
            }
          }
          break
        }
      }
      if (!supersummary) { print "S:", self_user }
      self_user_found = 1
      break
    }
    else if ($a == ">" && !self_user_found) {
      for (b=a+2;;b++) {
        if($b ~ /^[^a-zA-Z0-9_\-]?$/) {
          break
        }
        else {
          self_user = ((self_user) ($b))
        }
      }
      if (!supersummary) { print "S:", self_user }
      self_user_found = 1
      break
    }
  }

  if (onlyo && (onlyo != other_user)) {
    #print other_user, "SKIPPED!"
    next
  }

  if (self_user == other_user) {
    #print "EQUAL USER!"
    next
  }

  for (i=1;i!=NF;i++) {
    if (i <= 26 && !summary && !supersummary) {
      old_ors=ORS;ORS=""; if (!supersummary) { print $i; } ORS=old_ors
    }
    else if ($i == "<") {
      for(j=i+4;;j++) {
        if ($j == ".") {
          l=10
          for(k=j+1;;k++) {
            if ($k ~ /^[^0-9]/) {
              break
            }
            term = term + $k/l
            l=l*10
          }
          break
        }
        else if ($j ~ /^[^0-9]/) {
          break
        }
        else {
          term = term*10 + $j
        }
      }
      sum = sum - term;
      summinus = summinus + term;
      olistminus[other_user] += term;
      olist[other_user] -= term;
      if(!summary && !supersummary) { printf "%.2f %s %s\n", -1*term, "O:", other_user }
      next
    }
    else if($i == ">") {
      multiplier=1
      for(j=i-5;;j--) {
        if ($j == ".") {
          multiplier=1
          l=1
          for(k=j-1;;k--) {
            if ($k ~ /^[^0-9]/) {
              break
            }
            term = term + ($k)*l;
            l=l*10
          }
          break
        }
        else if ($j ~ /^[^0-9]/) {
          break
        }
        else {
          multiplier = multiplier*10
          term = (term + $j)/10
        }
      }
      term = term * multiplier
      sum = sum + term;
      sumplus = sumplus + term;
      olistplus[other_user] += term;
      olist[other_user] += term;
      if (!summary && !supersummary) { printf "%.2f %s %s\n", 1*term, "O:", other_user }
      next
    }
  }
}

END {
  if (!supersummary)
  {
    print "--USERLIST-START--"
    for (key in olist) {
      printf "%s %.2f %s %.2f %s %s\n", key, 1*olist[key], "Cr:", 1*olistplus[key], "Dr:", -1*olistminus[key]
    }
    print "---USERLIST-END---"
    printf "%s %.2f\n", "GRAND TOTAL:", 1*sum
    printf "%s %.2f\n", "CREDIT:", 1*sumplus
    printf "%s %.2f\n", "DEBIT:", -1*summinus
  }
  else
  {
    printf "%d\n", 100*sum
  }
}
