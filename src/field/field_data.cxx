
#include <globals.hxx>
#include <field_data.hxx>
#include <boundary_factory.hxx>
#include <output.hxx>
#include <field_factory.hxx>

FieldData::FieldData() : boundaryIsCopy(false), boundaryIsSet(true) {
  
}

FieldData::~FieldData() {
  if(!boundaryIsCopy) {
    // Delete the boundary operations
    for(const auto& bndry : bndry_op)
      delete bndry;
  }
}

void FieldData::setBoundary(const string &name) {
  /// Get the boundary factory (singleton)
  BoundaryFactory *bfact = BoundaryFactory::getInstance();
  
  output << "Setting boundary for variable " << name << endl;
  /// Loop over the mesh boundary regions
  for(const auto& reg : mesh->getBoundaries()) {
    BoundaryOp* op = static_cast<BoundaryOp*>(bfact->createFromOptions(name, reg));
    if(op != NULL)
      bndry_op.push_back(op);
    output << endl;
  }

  /// Get the mesh boundary regions
  vector<BoundaryRegionPar*> par_reg = mesh->getBoundariesPar();
  /// Loop over the mesh parallel boundary regions
  for(const auto& reg : mesh->getBoundariesPar()) {
    BoundaryOpPar* op = static_cast<BoundaryOpPar*>(bfact->createFromOptions(name, reg));
    if(op != NULL)
      bndry_op_par.push_back(op);
    output << endl;
  }

  boundaryIsSet = true;
  boundaryIsCopy = false;
}

void FieldData::setBoundary(const string &region, BoundaryOp *op) {
  /// Get the mesh boundary regions
  vector<BoundaryRegion*> reg = mesh->getBoundaries();
 
  /// Find the region
  

  /// Find if we're replacing an existing boundary
  for(const auto& bndry : bndry_op) {
    if( bndry->bndry == op->bndry ) {
      // Replacing this boundary
      output << "Replacing ";
    }
  }
}

void FieldData::copyBoundary(const FieldData &f) {
  bndry_op = f.bndry_op;
  bndry_op_par = f.bndry_op_par;
  boundaryIsCopy = true;
  boundaryIsSet = true;
}

//JMAD
void FieldData::addBndryFunction(FuncPtr userfunc, BndryLoc location){
  /// NOTE: This will allocate memory, which may never be free'd
  addBndryGenerator( new FieldFunction(userfunc), location );
}


void FieldData::addBndryGenerator(FieldGenerator* gen, BndryLoc location){
  if(location == BNDRY_ALL){
    for(const auto& reg : mesh->getBoundaries()) {
      bndry_generator[reg->location] = gen;
    }
  } else {
    bndry_generator[location] = gen;
  }
}

FieldGenerator* FieldData::getBndryGenerator(BndryLoc location) {
  std::map<BndryLoc,FieldGenerator*>::iterator it = bndry_generator.find(location);
  if(it == bndry_generator.end())
    return 0;
  
  return it->second;
}
