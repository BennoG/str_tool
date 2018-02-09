#!/bin/bash

#tar -xzf _tt.tgz

DLM=$(echo -e '\377')
FILS=$(find | egrep "\.c$|\.h$|\.inc$" | tr ' ' "${DLM}")

for FIL in $FILS; do
  FN=$(echo "$FIL" | tr "${DLM}" ' ')
  cat "$FN" |\
  grep -v "linuxams\.h" |\
  grep -v "amswin\.h" |\
  sed 's/ams_st\.h/stl_str\.h/g' |\
  sed 's/ams_tcp\.h/stl_tcp\.h/g' |\
  sed 's/ab_set_st/stlSetSt/g' |\
  sed 's/ab_get_st/stlGetStr/g' |\
  sed 's/ab_get_int/stlGetInt/g' |\
  sed 's/ab_store_st/stlStoreStr/g' |\
  sed 's/ab_store_int/stlStoreInt/g' |\
  sed 's/ab_dlm_get_st/stlGetDlm/g' |\
  sed 's/ab_insert_st/stlInsertStr/g' |\
  sed 's/ab_insert_int/stlInsertInt/g' |\
  sed 's/ab_locate_/stlLocate/g' |\
  sed 's/ab_cat_ch/stlAppendCh/g' |\
  sed 's/ab_cat_st/stlAppendSt/g' |\
  sed 's/ab_cat/stlAppend/g' |\
  sed 's/ab_copy/stlCopy/g' |\
  sed 's/bin_free/stlFree/g' |\
  sed 's/lxInitABP/stlInitLen/g' |\
  sed 's/AbConvert/stlConvert/g' |\
  sed 's/AbSwapStr/stlSwapStr/g' |\
  sed 's/StCount/stlCountChr/g' |\
  sed 's/abCountV/stlCountV/g' |\
  sed 's/ABP/STP/g' |\
  sed 's/_FM/_D1/g' |\
  sed 's/_VM/_D2/g' |\
  sed 's/_SVM/_D3/g' |\
  sed 's/->buf/->sBuf/g' |\
  sed 's/->clen/->iLen/g' |\
  sed 's/my_tcp_d/stlTcpConn/g' |\
  sed 's/myTcpInit/stlTcpInit/g' |\
  sed 's/myTcpRelease/stlTcpRelease/g' |\
  sed 's/myTcpConnect/stlTcpConnect/g' |\
  sed 's/myTcpGetChar/stlTcpGetChar/g' |\
  sed 's/myTcpWrite/stlTcpWrite/g' |\
  sed 's/MTaskWait/stlMsWait/g' |\
  sed 's/->aLine/->sLine/g' |\
  tr -d '\r' >"$FN.new"
  touch -r "$FN" "$FN.new"
  mv "$FN.new" "$FN"
done
