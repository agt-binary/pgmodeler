#include "constraint.h"

Constraint::Constraint(void)
{
 tabela_ref=NULL;
 obj_type=OBJ_CONSTRAINT;
 postergavel=false;
 fator_preenc=100;

 attributes[ParsersAttributes::PK_CONSTR]="";
 attributes[ParsersAttributes::FK_CONSTR]="";
 attributes[ParsersAttributes::CK_CONSTR]="";
 attributes[ParsersAttributes::UQ_CONSTR]="";
 attributes[ParsersAttributes::REF_TABLE]="";
 attributes[ParsersAttributes::SRC_COLUMNS]="";
 attributes[ParsersAttributes::DST_COLUMNS]="";
 attributes[ParsersAttributes::DEL_ACTION]="";
 attributes[ParsersAttributes::UPD_ACTION]="";
 attributes[ParsersAttributes::EXPRESSION]="";
 attributes[ParsersAttributes::TYPE]="";
 attributes[ParsersAttributes::COMPARISON_TYPE]="";
 attributes[ParsersAttributes::DEFER_TYPE]="";
 attributes[ParsersAttributes::DEFERRABLE]="";
 attributes[ParsersAttributes::TABLE]="";
 attributes[ParsersAttributes::DECL_IN_TABLE]="";
 attributes[ParsersAttributes::FACTOR]="";
}

Constraint::~Constraint(void)
{
 removerColunas();
}

void Constraint::definirTipo(TipoRestricao tipo)
{
 this->tipo=tipo;
}

void Constraint::definirTipoAcao(TipoAcao tipo, bool upd)
{
 //Se upd==true o tipo de ação no update é que será definido
 if(upd)
  this->acao_upd=tipo;
 else
  this->acao_del=tipo;
}

void Constraint::definirExpChecagem(const QString &exp)
{
 exp_checagem=exp;
}

bool Constraint::colunaExistente(Column *coluna, unsigned tipo_coluna)
{
 vector<Column *>::iterator itr, itr_end;
 Column *col_aux=NULL;
 bool enc=false;

 //Caso a coluna a ser buscada não esteja aloca, dispara uma exceção
 if(!coluna)
  throw Exception(ERR_OPR_NOT_ALOC_OBJECT,__PRETTY_FUNCTION__,__FILE__,__LINE__);

 if(tipo_coluna==COLUNA_ORIGEM)
 {
  itr=colunas.begin();
  itr_end=colunas.end();
 }
 else
 {
  itr=colunas_ref.begin();
  itr_end=colunas_ref.end();
 }

 /* Varre a lista de colunas selecionada verificando se o nome da
    coluna é igual ao nome de uma das colunas da lista ou mesmo
    se os endereços das colunas envolvidas são iguais */
 while(itr!=itr_end && !enc)
 {
  col_aux=(*itr);
  enc=(col_aux==coluna || col_aux->getName()==coluna->getName());
  itr++;
 }

 return(enc);
}

void Constraint::adicionarColuna(Column *coluna, unsigned tipo_coluna)
{
 //Caso a coluna não esteja aloca, dispara exceção.
 if(!coluna)
  throw Exception(Exception::getErrorMessage(ERR_ASG_NOT_ALOC_COLUMN)
                        .arg(QString::fromUtf8(this->getName()))
                        .arg(BaseObject::getTypeName(OBJ_CONSTRAINT)),
                 ERR_ASG_NOT_ALOC_COLUMN,__PRETTY_FUNCTION__,__FILE__,__LINE__);
 else if(tipo!=TipoRestricao::check)
 {
  //Só adiciona a coluna em uma das lista caso a mesma não exista em uma delas
  if(!colunaExistente(coluna,tipo_coluna))
  {
   //Caso a coluna a ser atribuida seja da lista de colunas de destino
   if(tipo_coluna==COLUNA_REFER)
    //Insere a coluna na lista de destino
    colunas_ref.push_back(coluna);
   else
    colunas.push_back(coluna);
  }
 }
}

void Constraint::setTablespace(Tablespace *espacotabela)
{
 try
 {
  if(espacotabela &&
     tipo!=TipoRestricao::primary_key &&
     tipo!=TipoRestricao::unique)
   throw Exception(ERR_ASG_TABSPC_INV_CONSTR_TYPE,__PRETTY_FUNCTION__,__FILE__,__LINE__);

  BaseObject::setTablespace(espacotabela);
 }
 catch(Exception &e)
 {
  throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__,&e);
 }
}

void Constraint::definirAtributoColunas(unsigned tipo_coluna, unsigned tipo_def, bool inc_insporrelacao)
{
 vector<Column *> *vet_col=NULL;
 Column *col;
 QString str_cols, atrib;
 unsigned i, qtd;
 bool formatar=(tipo_def==SchemaParser::SQL_DEFINITION);

 /* Caso a coluna selecionada seja a de destino,
    obtém a lista de colunas de destino e marca
    que o atributo a ser configurado é do de
    colunas da tabela de destino */
 if(tipo_coluna==COLUNA_REFER)
 {
  vet_col=&colunas_ref;
  atrib=ParsersAttributes::DST_COLUMNS;
 }
 else
 {
  vet_col=&colunas;
  atrib=ParsersAttributes::SRC_COLUMNS;
 }

 /* Varre a lista de colunas contatenando os nomes
    dos elementos separando-os por vírgula */
 qtd=vet_col->size();
 for(i=0; i < qtd; i++)
 {
  col=vet_col->at(i);

  /* No caso de definição XML as colunas protegidas (adicionaa   restrição
     por relacionamento) não podem ser incluídas pois estas serão inseridas
     na restrição no momento da criação do relacionamento a partir do XML respectivo
     por isso o parâmetro 'inc_insporrelacao' pode ser usado para resolver esse caso. */
  if((tipo_def==SchemaParser::SQL_DEFINITION) ||
     ((tipo_def==SchemaParser::XML_DEFINITION) &&
      ((inc_insporrelacao && col->isAddedByRelationship()) ||
       (inc_insporrelacao && !col->isAddedByRelationship()) ||
       (!inc_insporrelacao && !col->isAddedByRelationship()))))
  {
   str_cols+=col->getName(formatar);
   str_cols+=",";
  }
 }

 str_cols.remove(str_cols.size()-1,1);
 attributes[atrib]=str_cols;
}

void Constraint::definirTabReferenciada(BaseObject *tab_ref)
{
 this->tabela_ref=tab_ref;
}

void Constraint::definirTipoPostergacao(TipoPostergacao tipo)
{
 tipo_postergacao=tipo;
}

void Constraint::definirPostergavel(bool valor)
{
 postergavel=valor;
}

void Constraint::definirTipoComparacao(TipoComparacao tipo)
{
 tipo_comp=tipo;
}

void Constraint::definirFatorPreenchimento(unsigned fator)
{
 if(fator < 10) fator=10;
 fator_preenc=fator;
}

unsigned Constraint::obterFatorPreenchimento(void)
{
 return(fator_preenc);
}

TipoRestricao Constraint::obterTipoRestricao(void)
{
 return(tipo);
}

TipoAcao Constraint::obterTipoAcao(bool upd)
{
 //Se upd==true o tipo de ação no update é que será retornado
 if(upd)
  return(acao_upd);
 else
  return(acao_del);
}

QString Constraint::obterExpChecagem(void)
{
 return(exp_checagem);
}

Column *Constraint::obterColuna(unsigned idx_col, unsigned tipo_coluna)
{
 vector<Column *> *lista_col=NULL;

 //Obtendo a lista de colunas de acorodo com o tipo de coluna selecionado
 lista_col=(tipo_coluna==COLUNA_ORIGEM ? &colunas : &colunas_ref);

 /* Caso o índice de coluna a ser obtido seja inválido, um erro
    será retornado */
 if(idx_col>=lista_col->size())
  throw Exception(ERR_REF_COLUMN_INV_INDEX,__PRETTY_FUNCTION__,__FILE__,__LINE__);
 else
  //Retorna a coluna no índice especificado
  return(lista_col->at(idx_col));
}

Column *Constraint::obterColuna(const QString &nome, unsigned tipo_coluna)
{
 bool enc=false;
 vector<Column *> *lista_col=NULL;
 vector<Column *>::iterator itr_col, itr_end_col;

 //Obtém a lista de colunas de acordo com o tipo
 lista_col=(tipo_coluna==COLUNA_ORIGEM? &colunas : &colunas_ref);

 //Obtém as referencias para o primeiro e o ultimo elemento da lista de colunas
 itr_col=lista_col->begin();
 itr_end_col=lista_col->end();

 //Varre a lista de colunas verificando se existe alguma ocorrencia do nome passado
 while(itr_col!=itr_end_col)
 {
  enc=((*itr_col)->getName()==nome);
  //Caso o no não seja encontrado passa para o próximo elemento
  if(!enc) itr_col++;
  else break; //Caso seja encontrado, encerra a execuação da varredura
 }

 //caso seja encontrada, retorna a coluna senão retorna nulo
 if(enc) return(*itr_col);
 else return(NULL);
}

BaseObject *Constraint::obterTabReferenciada(void)
{
 return(tabela_ref);
}

unsigned Constraint::obterNumColunas(unsigned tipo_coluna)
{
 if(tipo_coluna==COLUNA_REFER)
  return(colunas_ref.size());
 else
  return(colunas.size());
}

void Constraint::removerColunas(void)
{
 colunas.clear();
 colunas_ref.clear();
}

void Constraint::removerColuna(const QString &nome, unsigned tipo_coluna)
{
 vector<Column *>::iterator itr, itr_end;
 vector<Column *> *cols=NULL;
 Column *col=NULL;

 //Se col_dest==true, a lista a ser pesquisada será a de destino
 if(tipo_coluna==COLUNA_REFER)
  //Selecionando a lista de destino para pesquisa
  cols=&colunas_ref;
 else
  cols=&colunas;

 //Obtém a referência ao primeiro elemento da lista selecionada
 itr=cols->begin();
 //Obtém a referência ao ultimo elemento da lista selecionada
 itr_end=cols->end();

 /* Efetua um iteração comparando o nome de cada coluna da lista
    com o nome que se deseja encontrar */
 while(itr!=itr_end)
 {
  col=(*itr); //Obtém a coluna

  //Caso o nome da coluna coincida com o nome a se localizar
  if(col->getName()==nome)
  {
   //Remove o elemento da lista
   cols->erase(itr);
   break;
  }
  else itr++; //Passa para a próxima coluna
 }
}

TipoPostergacao Constraint::obterTipoPostergacao(void)
{
 return(tipo_postergacao);
}

bool Constraint::restricaoPostergavel(void)
{
 return(postergavel);
}

bool Constraint::referenciaColunaIncRelacao(void)
{
 vector<Column *>::iterator itr, itr_end;
 Column *col=NULL;
 bool enc=false;

 /* Primeira lista de colunas da origem é que será varrida
    para isso as referências ao primeiro e ultimo elementos
    serão obtidas */
 itr=colunas.begin();
 itr_end=colunas.end();

 /* Efetua uma iteração verifica se as colunas da lista
    atual foram incluídas por relacionamento, caso uma coluna
    for detectada como incluída desta forma é suficiente
    dizer que a restrição referencia colunas vindas de
    relacionamento fazendo com que esta seja tratada de forma
    especial evitando a quebra de referêncais */
 while(itr!=itr_end && !enc)
 {
  //Obtém a coluna
  col=(*itr);
  //Obtém se a coluna foi incluída por relacionamento ou não
  enc=col->isAddedByRelationship();
  //Passa para a próxima coluna
  itr++;

  /* Caso a lista de colunas de origem foi completamente varrida
     e nenhuma das colunas desta vieram de relacionamentos, a
     lista de colunas referenciadas é que será varrida com o
     mesmo intuito */
  if(itr==itr_end && itr_end!=colunas_ref.end() && !enc)
  {
   itr=colunas_ref.begin();
   itr_end=colunas_ref.end();
  }
 }

 return(enc);
}

TipoComparacao Constraint::obterTipoComparacao(void)
{
 return(tipo_comp);
}

QString Constraint::getCodeDefinition(unsigned tipo_def)
{
 return(getCodeDefinition(tipo_def, false));
}

QString Constraint::getCodeDefinition(unsigned tipo_def, bool inc_insporrelacao)
{
 QString atrib;

 attributes[ParsersAttributes::PK_CONSTR]="";
 attributes[ParsersAttributes::FK_CONSTR]="";
 attributes[ParsersAttributes::CK_CONSTR]="";
 attributes[ParsersAttributes::UQ_CONSTR]="";

 switch(!tipo)
 {
  case TipoRestricao::check:
   atrib=ParsersAttributes::CK_CONSTR;
  break;
  case TipoRestricao::primary_key:
   atrib=ParsersAttributes::PK_CONSTR;
  break;
  case TipoRestricao::foreign_key:
   atrib=ParsersAttributes::FK_CONSTR;
  break;
  case TipoRestricao::unique:
   atrib=ParsersAttributes::UQ_CONSTR;
  break;
 }
 attributes[atrib]="1";

 attributes[ParsersAttributes::TYPE]=atrib;
 attributes[ParsersAttributes::UPD_ACTION]=(~acao_upd);
 attributes[ParsersAttributes::DEL_ACTION]=(~acao_del);
 attributes[ParsersAttributes::EXPRESSION]=exp_checagem;

 if(tipo!=TipoRestricao::check)
 {
  definirAtributoColunas(COLUNA_ORIGEM, tipo_def, inc_insporrelacao);

  /* Só gera a definição das colunas referenciadas da chave estrangeira
     caso o número de colunas da origem e destino sejam iguais, isso significa
     que a chave está configurada corretamente, caso contrário não gera o atributo
     forçando o parser de esquemas a retornar um erro pois a chave estrangeira está
     mal configurada. */
  if(tipo==TipoRestricao::foreign_key && colunas.size() == colunas_ref.size())
   definirAtributoColunas(COLUNA_REFER, tipo_def, inc_insporrelacao);
 }

 attributes[ParsersAttributes::REF_TABLE]=(tabela_ref ? tabela_ref->getName(true) : "");
 attributes[ParsersAttributes::DEFERRABLE]=(postergavel ? "1" : "");
 attributes[ParsersAttributes::COMPARISON_TYPE]=(~tipo_comp);
 attributes[ParsersAttributes::DEFER_TYPE]=(~tipo_postergacao);

 if(this->parent_table)
  attributes[ParsersAttributes::TABLE]=this->parent_table->getName(true);

 /* Caso a restrição não esteja referenciando alguma coluna incluída por relacionamento
    a mesma será declarada dentro do código da tabela pai e para tanto existe um atributo
    específico na definição SQL/XML do objeto chamado 'decl-in-table' que é usado
    para indicar ao parser quando a declaração da restrição está dentro da declaração da
    tabela pai. Este atributo é usado apenas para ajudar na formatação do código SQL e
    não tem nenhuma outra utilidade. */
 if(!referenciaColunaIncRelacao() || tipo==TipoRestricao::primary_key)
  attributes[ParsersAttributes::DECL_IN_TABLE]="1";

 if(tipo==TipoRestricao::primary_key || tipo==TipoRestricao::unique)
  attributes[ParsersAttributes::FACTOR]=QString("%1").arg(fator_preenc);
 else
  attributes[ParsersAttributes::FACTOR]="";

 return(BaseObject::__getCodeDefinition(tipo_def));
}
