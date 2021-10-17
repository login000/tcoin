#!/usr/bin/awk -f

#mst.awk -> message_sum_tcoin.awk

BEGIN {
  tsv=1
  summary=0
  sum=0
  sumplus=0
  summinus=0
  self_user=""
  self_user_found=0
  prev_self_txn=0
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
    if($1==" " && !prev_self_txn) {
      i=1;
      while($i != ":") { ++i }
      message_user = substr($0, 5, i-10); #position (i-5) - position (5) + 1 = length of message_user, where position (i-5) is the position of the last character of message_user
      old_ofs=OFS;OFS="";print "\n", message_user, "\t", substr($0,i+2);OFS=old_ofs;
    }
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
      if (!tsv) { print "S:", self_user }
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
      if (!tsv) { print "S:", self_user }
      self_user_found = 1
      break
    }
  }

  if (onlyo && (onlyo != other_user)) {
    #print other_user, "SKIPPED!"
    next
  }

  if (self_user == other_user) {
    prev_self_txn = 1
    #print "EQUAL USER!"
    next
  }
  else {
    prev_self_txn = 0
  }

  for (i=1;i!=NF;i++) {
    if (i <= 26 && !summary) {
      if(tsv && i == 25) {
        old_ors=ORS;ORS="";print "\t";ORS=old_ors
      }
      else if(tsv && i == 26) {
        #do nothing, don't print the "space"
      }
      else {
        old_ors=ORS;ORS="";print $i;ORS=old_ors
      }
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
      if(!summary) { if(tsv){ old_ofs=OFS;OFS="\t"; print other_user, self_user, -1*term; OFS=old_ofs } else { print -1*term, "O:", other_user } }
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
      if (!summary) {  if(tsv){ old_ofs=OFS;OFS="\t"; print other_user, self_user, term; OFS=old_ofs } else{ print 1*term, "O:", other_user } }
      next
    }
  }
}

END {
  if(!tsv) {
    print "--USERLIST-START--"
    for (key in olist) {
      print key, 1*olist[key], "Cr:", 1*olistplus[key], "Dr:", -1*olistminus[key]
    }
    print "---USERLIST-END---"
    print "GRAND TOTAL:", 1*sum
    print "CREDIT:", 1*sumplus
    print "DEBIT:", -1*summinus
  }
}
