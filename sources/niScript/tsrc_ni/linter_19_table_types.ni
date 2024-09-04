local __lint = {
  _all = 1
  _experimental = 1
  _pedantic = 0
}

function table_generic(table t) {
  t.name; // should fail
  t.not_a_field; // should fail
}

tThisFagiano <- {
  name = "vongole"
  im_fagiano = 0
}

function table_this_type(table:tThisFagiano t) {
  t.name; // should
  t.im_fagiano; // should be ok
  t.not_a_field; // should fail
}

qoo <- {
  tSubThisLion = {
    name = "simba"
    im_lion = 0
  }

  function table_in_table_sub(table:tSubThisLion t) {
    t.name; // should
    t.im_lion; // should be ok
    t.not_a_field; // should fail
  }
}

function table_this_sub_type(table:qoo.tSubThisLion t) {
  t.name; // should
  t.im_lion; // should be ok
  t.not_a_field; // should fail
}

::foo <- {
  tGlobalSquirrel = {
    name = "vongole"
    im_squirrel = 10000
  }
}

function table_global_type(::foo.tGlobalSquirrel t) {
  t.name; // should
  t.im_squirrel; // should be ok
  t.not_a_field; // should fail
}

/* TODO: Fetch type from local variables?

local tLocalHamster = {
  name = "spoochy"
  im_hamster = 3
}

function table_local_type(table:tLocalHamster t) {
  t.name; // should be ok
  t.im_hamster; // should be ok
}

*/
