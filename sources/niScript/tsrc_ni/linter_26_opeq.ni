::Import("niLang")

local __lint = {
  _all = 1
  _experimental = 1
  _pedantic = 0
}

abc <- 000;

function main() {
  xyz_eq = 123; // should fail, xyz doesnt exist
  xyz_pluseq += 123; // should fail, xyz doesnt exist
  xyz_minuseq -= 123; // should fail, xyz doesnt exist
  xyz_muleq *= 123; // should fail, xyz doesnt exist
  xyz_diveq /= 123; // should fail, xyz doesnt exist
  xyz_moduloeq %= 123; // should fail, xyz doesnt exist
  ++xyz_inc; // should fail, xyz doesnt exist
  xyz_pinc++; // should fail, xyz doesnt exist
  --xyz_dec; // should fail, xyz doesnt exist
  xyz_pdec--; // should fail, xyz doesnt exist
  xyz_shiftleq <<= 123; // should fail, xyz doesnt exist
  xyz_shiftreq >>= 123; // should fail, xyz doesnt exist
  xyz_ushiftreq >>>= 123; // should fail, xyz doesnt exist
  xyz_bwandeq &= 123; // should fail, xyz doesnt exist
  xyz_bworeq |= 123; // should fail, xyz doesnt exist
  xyz_bwxoreq ^= 123; // should fail, xyz doesnt exist

  abc = 123; // should succeed
  abc -= 123; // should succeed
  abc += 123; // should succeed
  abc *= 123; // should succeed
  abc /= 123; // should succeed
  abc %= 123; // should succeed
  abc >>= 123; // should succeed
  abc <<= 123; // should succeed
  abc >>>= 123; // should succeed
  abc &= 123; // should succeed
  abc |= 123; // should succeed
  abc ^= 123; // should succeed

  local def = 123;
  def = 123; // should succeed
  def -= 123; // should succeed
  def += 123; // should succeed
  def *= 123; // should succeed
  def /= 123; // should succeed
  def %= 123; // should succeed
  def >>= 123; // should succeed
  def <<= 123; // should succeed
  def >>>= 123; // should succeed
  def &= 123; // should succeed
  def |= 123; // should succeed
  def ^= 123; // should succeed
}
