
#include <inttypes.h>

#include "nsmtracker.h"
#include "visual_proc.h"
#include "hashmap_proc.h"

#include "Dynvec_proc.h"


static const dynvec_t g_empty_dynvec_dynvec = {0};
const dyn_t g_empty_dynvec = {
 .type = ARRAY_TYPE,
 .array = (dynvec_t*)&g_empty_dynvec_dynvec
};

const dyn_t g_uninitialized_dyn = {0};

bool DYNVEC_equal(dynvec_t *v1, dynvec_t *v2){

  if (v1->num_elements != v2->num_elements)
    return false;

  for(int i = 0 ; i<v1->num_elements ; i++)
    if (DYN_equal(v1->elements[i], v2->elements[i])==false)
      return false;

  return true;
}

void DYN_save(disk_t *file, const dyn_t dyn){
  DISK_printf(file,"%s\n",DYN_type_name(dyn.type));
  
  switch(dyn.type){
  case UNINITIALIZED_TYPE:
    RError("Uninitialized type not supported when saving hash to disk");
    break;
  case STRING_TYPE:
    DISK_write_wchar(file, dyn.string);
    DISK_write(file, "\n");
    break;
  case INT_TYPE:
    DISK_printf(file,"%" PRId64 "\n",dyn.int_number);
    break;
  case FLOAT_TYPE:
    DISK_printf(file,"%s\n",OS_get_string_from_double(dyn.float_number));
    break;
  case HASH_TYPE:
    if (dyn.hash==NULL)
      RError("element->hash==NULL");
    else
      HASH_save(dyn.hash, file);
    break;
  case ARRAY_TYPE:
    if (dyn.array==NULL)
      RError("dyn.array==NULL");
    else
      DYNVEC_save(file, *dyn.array);
    break;
  case RATIO_TYPE:
    DISK_printf(file,"%" PRId64 "\n",dyn.ratio->numerator);
    DISK_printf(file,"%" PRId64 "\n",dyn.ratio->denominator);
    break;
  case BOOL_TYPE:
    DISK_printf(file,"%d\n",dyn.bool_number ? 1 : 0);
    break;
  default:
    RError("Unknown type %d", dyn.type);
    return;
  }
}
  
void DYNVEC_save(disk_t *file, const dynvec_t dynvec){
  DISK_write(file, ">> DYNVEC BEGIN\n"); // Not really needed, but makes it simpler to read in a text editor. (it would be enough just to write version number)
  DISK_write(file, "3\n");
             
  DISK_printf(file, "%d\n", dynvec.num_elements);
  
  int i;
  for(i=0;i<dynvec.num_elements;i++)
    DYN_save(file, dynvec.elements[i]);
  
  DISK_write(file,"<< DYNVEC END\n");
}


static wchar_t *das_read_line(disk_t *file){

  wchar_t *line = DISK_read_wchar_line(file);

  //printf("%d: -%s-\n", curr_disk_line, STRING_get_chars(line));
  
  if(line==NULL){
    GFX_Message(NULL, "End of file before finished reading array");
    return NULL;
  }

  return line;
}

#define READ_LINE(file) das_read_line(file); if (line==NULL) return ret;

static int typename_to_type(const wchar_t *wtype_name){
  return DYN_get_type_from_name(STRING_get_chars(wtype_name));
}

dyn_t DYN_load(disk_t *file, bool *success){
  *success = false;

  dyn_t ret = {0};
  
  wchar_t *line = READ_LINE(file);

  int type = typename_to_type(line);

  //printf("           Putting %d / %s\n",i, key);
  switch(type){
  case UNINITIALIZED_TYPE:
    RError("UNINITIALIZED_TYPE?");
    break;
  case STRING_TYPE:
    line = READ_LINE(file);
    ret = DYN_create_string_dont_copy(line);
    break;
  case INT_TYPE:
    line = READ_LINE(file);
    ret = DYN_create_int(STRING_get_int64(line));
    break;
  case FLOAT_TYPE:
    line = READ_LINE(file);
    ret = DYN_create_float(STRING_get_double(line));
    break;
  case HASH_TYPE:
    {
      hash_t *hash = HASH_load(file);
      if (hash==NULL)
        return ret;
      ret = DYN_create_hash(hash);
      break;
    }
  case ARRAY_TYPE:
    {
      dynvec_t dynvec = DYNVEC_load(file, success);
      if(*success==false)
        return ret;
      else
        *success = false;
      ret = DYN_create_array(dynvec);
    }
    break;
  case RATIO_TYPE:
    {
      line = READ_LINE(file);
      int64_t numerator = STRING_get_int64(line);
      line = READ_LINE(file);
      int64_t denominator = STRING_get_int64(line);
      ret = DYN_create_ratio(make_ratio(numerator, denominator));
    }
    break;
  case BOOL_TYPE:
    line = READ_LINE(file);
    ret = DYN_create_bool(STRING_get_int(line)==1 ? true : false);
    break;
  default:
    RError("Unknown type %d", type);
    return ret;
  }

  *success = true;

  return ret;
}

dynvec_t DYNVEC_load(disk_t *file, bool *success){
  dynvec_t ret = {0};

  *success = false;
  
  wchar_t *line = READ_LINE(file);

  if (!STRING_equals(line,">> DYNVEC BEGIN")){
    GFX_Message(NULL, "Trying to load something which is not an array. Expected \"%s\", found \"%s\"",">> DYNVEC BEGIN",STRING_get_chars(line));
    return ret;
  }

  line = READ_LINE(file);
  
  int version = STRING_get_int(line);

  if (version != 3){
    vector_t v = {0};
    int try_anyway = VECTOR_push_back(&v, "Try anyway (on your own risk)");
    (void)try_anyway;
    int ok = VECTOR_push_back(&v, "Ok");
    int res = GFX_Message(&v, "Need a newer version or Radium to load this file");
    if (res==ok)
      return ret;
  }
  
  line = READ_LINE(file);
  
  int elements_size = STRING_get_int(line);

  for(int i = 0 ; i < elements_size ; i++){
    dyn_t dyn = DYN_load(file, success);
    if (*success==false)
      return ret;
    else
      *success = false;

    DYNVEC_push_back(&ret, dyn);
  }

  line = READ_LINE(file);
  
  if(!STRING_equals(line,"<< DYNVEC END")){
    GFX_Message(NULL, "Something went wrong when loading array of size %d from disk. Expected \"%s\", but found \"%s\".", "<< DYNVEC END", STRING_get_chars(line));
    return ret;
  }
  
  *success = true;
  
  return ret;
}
