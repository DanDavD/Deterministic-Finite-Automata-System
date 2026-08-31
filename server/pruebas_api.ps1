$ErrorActionPreference = 'Continue'
$base = 'http://localhost:8080'

$script:pasadas = 0
$script:fallidas = 0
$script:fallos = @()
$script:seccion = ''

function Seccion($n) { $script:seccion = $n; Write-Host ""; Write-Host "=== $n ===" }

function Chk($nombre, $cond, $detalle) {
  if ($cond) {
    $script:pasadas++
  } else {
    $script:fallidas++
    $script:fallos += "[$script:seccion] $nombre :: $detalle"
    Write-Host ("  FALLO  " + $nombre + "  ->  " + $detalle) -ForegroundColor Red
  }
}

function Enc($s) { return [System.Uri]::EscapeDataString([string]$s) }

function Pedir($metodo, $ruta, $cuerpo) {
  try {
    if ($metodo -eq 'GET') {
      $r = Invoke-WebRequest -Uri ($base + $ruta) -UseBasicParsing -TimeoutSec 15
    } else {
      $r = Invoke-WebRequest -Uri ($base + $ruta) -Method Post -Body $cuerpo -ContentType 'application/x-www-form-urlencoded' -UseBasicParsing -TimeoutSec 15
    }
    $j = $null
    try { $j = $r.Content | ConvertFrom-Json } catch {}
    return @{ status = [int]$r.StatusCode; body = $j; raw = $r.Content; muerto = $false }
  } catch {
    $resp = $_.Exception.Response
    if ($null -eq $resp) {
      return @{ status = 0; body = $null; raw = $_.Exception.Message; muerto = $true }
    }
    $code = [int]$resp.StatusCode
    $texto = ''
    try {
      $sr = New-Object System.IO.StreamReader($resp.GetResponseStream())
      $texto = $sr.ReadToEnd()
    } catch {}
    $j = $null
    if ($texto) { try { $j = $texto | ConvertFrom-Json } catch {} }
    return @{ status = $code; body = $j; raw = $texto; muerto = $false }
  }
}

function Post($ruta, $cuerpo) { return Pedir 'POST' $ruta $cuerpo }
function Get_($ruta) { return Pedir 'GET' $ruta $null }

$SEP = [string][char]31

# los casos de este archivo se escribieron con "origen|simbolo|destino". el
# protocolo real usa un separador de control, asi que se traduce solo lo que
# tiene la forma correcta; lo mal formado se manda tal cual para que el servidor
# lo rechace, que es justamente lo que esos casos quieren comprobar.
function ConvertirTransicion($t) {
  $partes = $t -split '\|'
  if (($partes.Count -eq 3) -and ($partes[1].Length -eq 1)) {
    return $partes[0] + $SEP + $partes[1] + $SEP + $partes[2]
  }
  return $t
}

function Crear($nombre, $estados, $simbolos, $inicial, $finales, $transiciones, $guardar) {
  $b = 'nombre=' + (Enc $nombre)
  foreach ($e in $estados)      { $b += '&estado='     + (Enc $e) }
  foreach ($s in $simbolos)     { $b += '&simbolo='    + (Enc $s) }
  if ($null -ne $inicial)       { $b += '&inicial='    + (Enc $inicial) }
  foreach ($f in $finales)      { $b += '&final='      + (Enc $f) }
  foreach ($t in $transiciones) { $b += '&transicion=' + (Enc (ConvertirTransicion $t)) }
  $b += '&guardar=' + $guardar
  return Post '/api/automatas' $b
}

function Vivo() {
  $r = Get_ '/api/automatas'
  return (-not $r.muerto) -and ($r.status -eq 200)
}

Write-Host "Servidor vivo al arrancar: $(Vivo)"

# =====================================================================
Seccion 'A. AUTOMATAS VALIDOS BASICOS'
# =====================================================================

$A1 = Crear 'TerminaEnA' @('q0','q1') @('a','b') 'q0' @('q1') @('q0|a|q1','q0|b|q0','q1|a|q1','q1|b|q0') 1
Chk 'A1 valido' ($A1.body.valido -eq $true) "valido=$($A1.body.valido) errores=$($A1.body.errores -join '; ')"
Chk 'A1 guardado' ($A1.body.guardado -eq $true) "guardado=$($A1.body.guardado)"
$idA1 = $A1.body.id

$A2 = Crear 'ParesDeB' @('p0','p1') @('a','b') 'p0' @('p0') @('p0|a|p0','p0|b|p1','p1|a|p1','p1|b|p0') 1
Chk 'A2 valido' ($A2.body.valido -eq $true) "errores=$($A2.body.errores -join '; ')"
$idA2 = $A2.body.id

# un solo estado, acepta todo
$A3 = Crear 'UnEstadoTodo' @('u0') @('a','b') 'u0' @('u0') @('u0|a|u0','u0|b|u0') 1
Chk 'A3 un estado valido' ($A3.body.valido -eq $true) "errores=$($A3.body.errores -join '; ')"
$idA3 = $A3.body.id

# conjunto de finales vacio: el enunciado dice que es VALIDO
$A4 = Crear 'SinFinales' @('s0','s1') @('a','b') 's0' @() @('s0|a|s1','s0|b|s0','s1|a|s1','s1|b|s0') 1
Chk 'A4 sin finales es valido' ($A4.body.valido -eq $true) "errores=$($A4.body.errores -join '; ')"
Chk 'A4 finales vacio' ($A4.body.automata.finales.Count -eq 0) "finales=$($A4.body.automata.finales.Count)"
$idA4 = $A4.body.id

# alfabeto vacio: la rubrica pide no vacuidad, asi que se rechaza
$A5 = Crear 'SinSimbolos' @('z0') @() 'z0' @('z0') @() 1
Chk 'A5 alfabeto vacio no revienta' (-not $A5.muerto) 'servidor no respondio'
Chk 'A5 alfabeto vacio invalido' ($A5.body.valido -eq $false) "valido=$($A5.body.valido)"
Chk 'A5 error nombra el alfabeto vacio' (($A5.body.errores -join ' ') -like '*alfabeto esta vacio*') "errores=$($A5.body.errores -join '; ')"
Chk 'A5 no se guarda' ($A5.body.guardado -eq $false) "guardado=$($A5.body.guardado)"
$idA5 = $A5.body.id

# conjunto de estados vacio: mismo criterio
$A6 = Crear 'SinEstados' @() @('a') '' @() @() 1
Chk 'A6 sin estados invalido' ($A6.body.valido -eq $false) "valido=$($A6.body.valido)"
Chk 'A6 error nombra el conjunto vacio' (($A6.body.errores -join ' ') -like '*estados esta vacio*') "errores=$($A6.body.errores -join '; ')"

# =====================================================================
Seccion 'B. VALIDACION: CASOS INVALIDOS'
# =====================================================================

$B1 = Crear 'SinInicial' @('q0','q1') @('a') $null @('q1') @('q0|a|q1','q1|a|q1') 1
Chk 'B1 sin inicial invalido' ($B1.body.valido -eq $false) "valido=$($B1.body.valido)"
Chk 'B1 no se guarda' ($B1.body.guardado -eq $false) "guardado=$($B1.body.guardado)"

$B2 = Crear 'InicialFuera' @('q0','q1') @('a') 'zz' @('q1') @('q0|a|q1','q1|a|q1') 1
Chk 'B2 inicial fuera de Q invalido' ($B2.body.valido -eq $false) 'deberia ser invalido'
Chk 'B2 error menciona zz' (($B2.body.errores -join ' ') -like '*zz*') "errores=$($B2.body.errores -join '; ')"

$B3 = Crear 'DeltaIncompleta' @('q0','q1') @('a','b') 'q0' @('q1') @('q0|a|q1','q1|a|q1','q1|b|q0') 1
Chk 'B3 delta incompleta invalida' ($B3.body.valido -eq $false) 'deberia ser invalido'
Chk 'B3 error nombra estado y simbolo' (($B3.body.errores -join ' ') -like "*q0*b*") "errores=$($B3.body.errores -join '; ')"

$B4 = Crear 'NoDeterminista' @('q0','q1') @('a') 'q0' @('q1') @('q0|a|q0','q0|a|q1','q1|a|q1') 1
Chk 'B4 no determinismo invalido' ($B4.body.valido -eq $false) 'deberia ser invalido'
Chk 'B4 error habla de mas de una transicion' (($B4.body.errores -join ' ') -like '*mas de una*') "errores=$($B4.body.errores -join '; ')"

$B5 = Crear 'DestinoFantasma' @('q0') @('a') 'q0' @() @('q0|a|q9') 1
Chk 'B5 destino no registrado invalido' ($B5.body.valido -eq $false) 'deberia ser invalido'
Chk 'B5 error menciona q9' (($B5.body.errores -join ' ') -like '*q9*') "errores=$($B5.body.errores -join '; ')"

$B6 = Crear 'TodoRoto' @('r0','r1') @('a','b') 'rX' @('rY') @('r0|a|rZ','r0|a|r1') 1
Chk 'B6 multiples errores a la vez' ($B6.body.errores.Count -ge 3) "cantidad errores=$($B6.body.errores.Count)"
Chk 'B6 no se guarda' ($B6.body.guardado -eq $false) 'no debe guardarse'

# validar sin guardar
$B7 = Crear 'SoloValidar' @('q0') @('a') 'q0' @() @('q0|a|q0') 0
Chk 'B7 valido pero no guardado' (($B7.body.valido -eq $true) -and ($B7.body.guardado -eq $false)) "valido=$($B7.body.valido) guardado=$($B7.body.guardado)"

# =====================================================================
Seccion 'C. ENTRADAS SUCIAS: LAS REPORTA EL VALIDADOR, NO LA INSERCION'
# =====================================================================

# httplib colapsa los pares clave=valor identicos, asi que el duplicado exacto
# ni siquiera llega al modelo. lo que importa es que el resultado sea correcto.
$C1 = Crear 'EstadoDup' @('q0','q0','q1') @('a') 'q0' @('q1') @('q0|a|q1','q1|a|q1') 1
Chk 'C1 quedan 2 estados' ($C1.body.automata.estados.Count -eq 2) "estados=$($C1.body.automata.estados.Count)"
Chk 'C1 valido pese al duplicado' ($C1.body.valido -eq $true) "errores=$($C1.body.errores -join '; ')"

$C2 = Crear 'SimboloDup' @('q0') @('a','a') 'q0' @() @('q0|a|q0') 1
Chk 'C2 queda 1 simbolo' ($C2.body.automata.alfabeto.Count -eq 1) "alfabeto=$($C2.body.automata.alfabeto.Count)"
Chk 'C2 valido pese al duplicado' ($C2.body.valido -eq $true) "errores=$($C2.body.errores -join '; ')"

# duplicado que si llega distinto al modelo: mismo estado con otra capitalizacion no colisiona
$C2b = Crear 'CasiDup' @('q0','Q0') @('a') 'q0' @() @('q0|a|Q0','Q0|a|q0') 1
Chk 'C2b q0 y Q0 son distintos' ($C2b.body.automata.estados.Count -eq 2) "estados=$($C2b.body.automata.estados -join ',')"

# el espacio entra al alfabeto y es el validador el que lo rechaza, nombrandolo
$C3 = Crear 'SimboloEspacio' @('q0') @(' ','a') 'q0' @() @('q0|a|q0') 1
Chk 'C3 espacio entra al alfabeto' ($C3.body.automata.alfabeto.Count -eq 2) "alfabeto=[$($C3.body.automata.alfabeto -join '')]"
Chk 'C3 espacio invalida el automata' ($C3.body.valido -eq $false) "valido=$($C3.body.valido)"
Chk 'C3 error nombra el espacio en blanco' (($C3.body.errores -join ' ') -like '*espacio en blanco*') "errores=$($C3.body.errores -join '; ')"
Chk 'C3 no se guarda' ($C3.body.guardado -eq $false) "guardado=$($C3.body.guardado)"

$C4 = Crear 'SimboloGuion' @('q0') @('-','a') 'q0' @() @('q0|a|q0') 1
Chk 'C4 guion entra al alfabeto' ($C4.body.automata.alfabeto.Count -eq 2) "alfabeto=$($C4.body.automata.alfabeto -join ',')"
Chk 'C4 guion invalida el automata' ($C4.body.valido -eq $false) "valido=$($C4.body.valido)"
Chk 'C4 error nombra el guion' (($C4.body.errores -join ' ') -like "*guion*") "errores=$($C4.body.errores -join '; ')"

# epsilon y lambda son multibyte: no caben en el char del alfabeto, se frenan
# al leer la entrada y el aviso tiene que decir por que
$C5 = Crear 'SimboloEpsilon' @('q0') @([char]0x03B5, 'a') 'q0' @() @('q0|a|q0') 1
Chk 'C5 epsilon multibyte rechazado' ($C5.body.automata.alfabeto.Count -eq 1) "alfabeto=$($C5.body.automata.alfabeto -join ',')"
Chk 'C5 avisa del epsilon' ($C5.body.avisos.Count -ge 1) "avisos=$($C5.body.avisos -join '; ')"
Chk 'C5 el aviso lo llama epsilon' (($C5.body.avisos -join ' ') -like '*epsilon*') "avisos=$($C5.body.avisos -join '; ')"
Chk 'C5 el aviso dice que es reservado' (($C5.body.avisos -join ' ') -like '*reservado*') "avisos=$($C5.body.avisos -join '; ')"

$C6 = Crear 'SimboloLambda' @('q0') @([char]0x03BB, 'a') 'q0' @() @('q0|a|q0') 1
Chk 'C6 lambda multibyte rechazado' ($C6.body.automata.alfabeto.Count -eq 1) "alfabeto=$($C6.body.automata.alfabeto -join ',')"
Chk 'C6 el aviso lo llama lambda' (($C6.body.avisos -join ' ') -like '*lambda*') "avisos=$($C6.body.avisos -join '; ')"

# el final inexistente queda registrado y lo denuncia el validador
$C7 = Crear 'FinalFuera' @('q0') @('a') 'q0' @('noExiste') @('q0|a|q0') 1
Chk 'C7 final inexistente queda registrado' ($C7.body.automata.finales -contains 'noExiste') "finales=$($C7.body.automata.finales -join ',')"
Chk 'C7 final inexistente invalida' ($C7.body.valido -eq $false) "valido=$($C7.body.valido)"
Chk 'C7 error menciona noExiste' (($C7.body.errores -join ' ') -like '*noExiste*') "errores=$($C7.body.errores -join '; ')"
Chk 'C7 no se guarda' ($C7.body.guardado -eq $false) "guardado=$($C7.body.guardado)"

$C8 = Crear 'SimboloTab' @('q0') @("`t", 'a') 'q0' @() @('q0|a|q0') 1
Chk 'C8 tab entra al alfabeto' ($C8.body.automata.alfabeto.Count -eq 2) "alfabeto count=$($C8.body.automata.alfabeto.Count)"
Chk 'C8 tab invalida el automata' ($C8.body.valido -eq $false) "valido=$($C8.body.valido)"
Chk 'C8 error nombra la tabulacion' (($C8.body.errores -join ' ') -like '*tabulacion*') "errores=$($C8.body.errores -join '; ')"

$C9 = Crear 'SimboloLargo' @('q0') @('ab','a') 'q0' @() @('q0|a|q0') 1
Chk 'C9 simbolo de 2 chars rechazado' ($C9.body.automata.alfabeto.Count -eq 1) "alfabeto=$($C9.body.automata.alfabeto -join ',')"

# =====================================================================
Seccion 'D. NOMBRES Y SIMBOLOS HOSTILES'
# =====================================================================

$D1 = Crear 'Comillas' @('q"0','q\1') @('a') 'q"0' @('q\1') @('q"0|a|q\1','q\1|a|q\1') 1
Chk 'D1 comillas y backslash: json valido' ($null -ne $D1.body) "raw=$($D1.raw)"
Chk 'D1 automata valido' ($D1.body.valido -eq $true) "errores=$($D1.body.errores -join '; ')"
Chk 'D1 nombres preservados' ($D1.body.automata.estados -contains 'q"0') "estados=$($D1.body.automata.estados -join ' | ')"
$idD1 = $D1.body.id

$D2 = Crear 'Script' @('<script>alert(1)</script>','q1') @('a') '<script>alert(1)</script>' @('q1') @('<script>alert(1)</script>|a|q1','q1|a|q1') 1
Chk 'D2 html en nombres: json valido' ($null -ne $D2.body) 'json no parseo'
Chk 'D2 valido' ($D2.body.valido -eq $true) "errores=$($D2.body.errores -join '; ')"

# ojo: hay que forzar [string] o PowerShell concatena todo el arreglo en un solo elemento
$u1 = [string]([char]0x00F1) + 'x'
$u2 = [string]([char]0x4E2D) + 'y'
$D3 = Crear 'Unicode' @($u1, $u2) @('a') $u1 @() @("$u1|a|$u2", "$u2|a|$u2") 1
Chk 'D3 unicode en estados no revienta' (-not $D3.muerto) 'servidor cayo'
Chk 'D3 json valido' ($null -ne $D3.body) 'json no parseo'
Chk 'D3 dos estados unicode registrados' ($D3.body.automata.estados.Count -eq 2) "estados=$($D3.body.automata.estados -join ',')"
Chk 'D3 valido' ($D3.body.valido -eq $true) "errores=$($D3.body.errores -join '; ')"
Chk 'D3 enie intacta' ($D3.body.automata.estados -contains $u1) "esperaba '$u1' en [$($D3.body.automata.estados -join ' , ')]"
Chk 'D3 cjk intacto' ($D3.body.automata.estados -contains $u2) "esperaba '$u2'"

$nombreLargo = 'q' + ('x' * 300)
$D4 = Crear 'NombreLargo' @($nombreLargo,'q1') @('a') $nombreLargo @('q1') @("$nombreLargo|a|q1","q1|a|q1") 1
Chk 'D4 nombre de 300 chars no revienta' (-not $D4.muerto) 'servidor cayo'
Chk 'D4 valido' ($D4.body.valido -eq $true) "errores=$($D4.body.errores -join '; ')"
Chk 'D4 nombre completo preservado' (($D4.body.automata.estados -contains $nombreLargo)) 'se trunco el nombre'

$D5 = Crear 'ConPipe' @('q|0','q1') @('a') 'q|0' @('q1') @('q|0|a|q1','q1|a|q1') 1
Chk 'D5 pipe en nombre no revienta' (-not $D5.muerto) 'servidor cayo'
Write-Host "  (info) D5 pipe en estado -> valido=$($D5.body.valido) avisos=$($D5.body.avisos.Count) errores=$($D5.body.errores.Count)"

$D6 = Crear 'SimbolosURL' @('q0') @('&','=') 'q0' @() @('q0|&|q0','q0|=|q0') 1
Chk 'D6 simbolos & y = aceptados' ($D6.body.automata.alfabeto.Count -eq 2) "alfabeto=$($D6.body.automata.alfabeto -join ',')"
Chk 'D6 valido' ($D6.body.valido -eq $true) "errores=$($D6.body.errores -join '; ')"

$D7 = Crear 'SimbolosMas' @('q0') @('+','%') 'q0' @() @('q0|+|q0','q0|%|q0') 1
Chk 'D7 simbolos + y % aceptados' ($D7.body.automata.alfabeto.Count -eq 2) "alfabeto=$($D7.body.automata.alfabeto -join ',')"
Chk 'D7 valido' ($D7.body.valido -eq $true) "errores=$($D7.body.errores -join '; ')"

$D8 = Crear 'MayusMinus' @('q0','q1') @('a','A') 'q0' @('q1') @('q0|a|q0','q0|A|q1','q1|a|q1','q1|A|q1') 1
Chk 'D8 a y A son distintos' ($D8.body.automata.alfabeto.Count -eq 2) "alfabeto=$($D8.body.automata.alfabeto -join ',')"
Chk 'D8 valido' ($D8.body.valido -eq $true) "errores=$($D8.body.errores -join '; ')"
$idD8 = $D8.body.id

$D9 = Crear '' @('q0') @('a') 'q0' @() @('q0|a|q0') 1
Chk 'D9 nombre de automata vacio no revienta' (-not $D9.muerto) 'servidor cayo'
Chk 'D9 valido' ($D9.body.valido -eq $true) "errores=$($D9.body.errores -join '; ')"

$D10 = Crear 'EstadoVacio' @('','q1') @('a') 'q1' @() @('q1|a|q1') 1
Chk 'D10 estado vacio ignorado' ($D10.body.automata.estados.Count -eq 1) "estados=$($D10.body.automata.estados.Count)"

$D11 = Crear 'NombreConEspacios' @('mi estado','q1') @('a') 'mi estado' @('q1') @('mi estado|a|q1','q1|a|q1') 1
Chk 'D11 espacios en nombre de estado' ($D11.body.valido -eq $true) "errores=$($D11.body.errores -join '; ')"
Chk 'D11 nombre con espacio preservado' ($D11.body.automata.estados -contains 'mi estado') "estados=$($D11.body.automata.estados -join '|')"

# =====================================================================
Seccion 'E. TRANSICIONES MAL FORMADAS'
# =====================================================================

$E1 = Crear 'TransSinPipes' @('q0') @('a') 'q0' @() @('q0aq0') 1
Chk 'E1 transicion sin pipes se ignora con aviso' ($E1.body.avisos.Count -ge 1) "avisos=$($E1.body.avisos -join '; ')"
Chk 'E1 queda invalida por delta incompleta' ($E1.body.valido -eq $false) 'deberia faltar la transicion'

$E2 = Crear 'TransUnPipe' @('q0') @('a') 'q0' @() @('q0|a') 1
Chk 'E2 un solo pipe ignorado' ($E2.body.avisos.Count -ge 1) "avisos=$($E2.body.avisos -join '; ')"

$E3 = Crear 'TransSimboloLargo' @('q0') @('a') 'q0' @() @('q0|ab|q0') 1
Chk 'E3 simbolo de 2 chars en transicion ignorado' ($E3.body.avisos.Count -ge 1) "avisos=$($E3.body.avisos -join '; ')"

$E4 = Crear 'TransOrigenVacio' @('q0') @('a') 'q0' @() @('|a|q0') 1
Chk 'E4 origen vacio ignorado' ($E4.body.avisos.Count -ge 1) "avisos=$($E4.body.avisos -join '; ')"

$E5 = Crear 'TransDestinoVacio' @('q0') @('a') 'q0' @() @('q0|a|') 1
Chk 'E5 destino vacio ignorado' ($E5.body.avisos.Count -ge 1) "avisos=$($E5.body.avisos -join '; ')"

$E6 = Crear 'TransMuchosPipes' @('q0') @('a') 'q0' @() @('q0|a|q0|extra') 1
Chk 'E6 pipes de mas ignorado' ($E6.body.avisos.Count -ge 1) "avisos=$($E6.body.avisos -join '; ')"

$E7 = Crear 'TransVacia' @('q0') @('a') 'q0' @() @('','q0|a|q0') 1
Chk 'E7 transicion vacia saltada sin romper' ($E7.body.valido -eq $true) "errores=$($E7.body.errores -join '; ')"

Chk 'E servidor sigue vivo' (Vivo) 'el servidor murio en la seccion E'

# =====================================================================
Seccion 'F. UNIONES'
# =====================================================================

$F1 = Post '/api/unir' "idA=$idA1&idB=$idA2"
Chk 'F1 union basica exitosa' ($F1.body.exitoso -eq $true) "error=$($F1.body.error)"
Chk 'F1 tiene 4 estados' ($F1.body.automata.estados.Count -eq 4) "estados=$($F1.body.automata.estados.Count)"
Chk 'F1 inicial es el par' ($F1.body.automata.inicial -eq '(q0,p0)') "inicial=$($F1.body.automata.inicial)"
Chk 'F1 finales por OR' ($F1.body.automata.finales.Count -eq 3) "finales=$($F1.body.automata.finales -join ',')"
Chk 'F1 8 transiciones' ($F1.body.automata.transiciones.Count -eq 8) "transiciones=$($F1.body.automata.transiciones.Count)"
Chk 'F1 construccion tiene 4 pasos' ($F1.body.construccion.pasos.Count -eq 4) "pasos=$($F1.body.construccion.pasos.Count)"
$idU1 = $F1.body.id

# coherencia derivacion vs automata real
$coherente = $true
$detalleCoh = ''
foreach ($paso in $F1.body.construccion.pasos) {
  foreach ($t in $paso.transiciones) {
    $real = $F1.body.automata.transiciones | Where-Object { $_.origen -eq $paso.par -and $_.simbolo -eq $t.simbolo }
    if ($real.destino -ne $t.destino) { $coherente = $false; $detalleCoh += "$($paso.par),$($t.simbolo) "; }
  }
  $esFinalReal = $F1.body.automata.finales -contains $paso.par
  if ($esFinalReal -ne $paso.esFinal) { $coherente = $false; $detalleCoh += "final:$($paso.par) " }
}
Chk 'F1 derivacion coincide con el nucleo' $coherente "discrepancias: $detalleCoh"

# union consigo mismo
$F2 = Post '/api/unir' "idA=$idA1&idB=$idA1"
Chk 'F2 union consigo mismo exitosa' ($F2.body.exitoso -eq $true) "error=$($F2.body.error)"
Chk 'F2 tiene 4 estados' ($F2.body.automata.estados.Count -eq 4) "estados=$($F2.body.automata.estados.Count)"
$idU2 = $F2.body.id

# union anidada
$F3 = Post '/api/unir' "idA=$idU1&idB=$idA1"
Chk 'F3 union de union exitosa' ($F3.body.exitoso -eq $true) "error=$($F3.body.error)"
Chk 'F3 tiene 8 estados' ($F3.body.automata.estados.Count -eq 8) "estados=$($F3.body.automata.estados.Count)"
Chk 'F3 nombres anidados' ($F3.body.automata.inicial -eq '((q0,p0),q0)') "inicial=$($F3.body.automata.inicial)"
$idU3 = $F3.body.id

# triple anidamiento
$F4 = Post '/api/unir' "idA=$idU3&idB=$idA2"
Chk 'F4 triple anidamiento' ($F4.body.exitoso -eq $true) "error=$($F4.body.error)"
Chk 'F4 tiene 16 estados' ($F4.body.automata.estados.Count -eq 16) "estados=$($F4.body.automata.estados.Count)"
$idU4 = $F4.body.id

# alfabetos distintos
$G1 = Crear 'AlfabetoAC' @('w0') @('a','c') 'w0' @('w0') @('w0|a|w0','w0|c|w0') 1
$idG1 = $G1.body.id
$F5 = Post '/api/unir' "idA=$idA1&idB=$idG1"
Chk 'F5 alfabetos distintos bloqueado' ($F5.body.exitoso -eq $false) "exitoso=$($F5.body.exitoso)"
Chk 'F5 error nombra los simbolos' (($F5.body.error -like '*b*') -and ($F5.body.error -like '*c*')) "error=$($F5.body.error)"

# alfabeto en orden distinto pero mismo conjunto
$G2 = Crear 'OrdenInvertido' @('v0','v1') @('b','a') 'v0' @('v1') @('v0|a|v1','v0|b|v0','v1|a|v1','v1|b|v0') 1
$idG2 = $G2.body.id
$F6 = Post '/api/unir' "idA=$idA1&idB=$idG2"
Chk 'F6 mismo alfabeto distinto orden se une' ($F6.body.exitoso -eq $true) "error=$($F6.body.error)"

# el de alfabeto vacio no paso la validacion, asi que no se guardo: el enunciado
# pide que un automata invalido tampoco se pueda usar en operaciones posteriores
$F7 = Post '/api/unir' "idA=$idA5&idB=$idA1"
Chk 'F7 un automata invalido no se puede unir' ($F7.status -eq 400) "status=$($F7.status) idA5=$idA5"

$F8 = Post '/api/unir' "idA=$idA5&idB=$idA5"
Chk 'F8 dos invalidos tampoco' ($F8.status -eq 400) "status=$($F8.status)"
Chk 'F8 servidor sigue vivo' (-not $F8.muerto) 'servidor cayo'

# union con el de un estado
$F9 = Post '/api/unir' "idA=$idA3&idB=$idA1"
Chk 'F9 union 1x2' ($F9.body.exitoso -eq $true) "error=$($F9.body.error)"
Chk 'F9 tiene 2 estados' ($F9.body.automata.estados.Count -eq 2) "estados=$($F9.body.automata.estados.Count)"
Chk 'F9 todos finales (A3 acepta todo)' ($F9.body.automata.finales.Count -eq 2) "finales=$($F9.body.automata.finales.Count)"
$idF9 = $F9.body.id

# union con sin-finales
$F10 = Post '/api/unir' "idA=$idA4&idB=$idA4"
Chk 'F10 union de dos sin finales' ($F10.body.exitoso -eq $true) "error=$($F10.body.error)"
Chk 'F10 sigue sin finales' ($F10.body.automata.finales.Count -eq 0) "finales=$($F10.body.automata.finales.Count)"

# union con nombres hostiles
$F11 = Post '/api/unir' "idA=$idD1&idB=$idD1"
Chk 'F11 union con comillas en nombres' ($F11.body.exitoso -eq $true) "error=$($F11.body.error)"
Chk 'F11 json valido' ($null -ne $F11.body.automata) 'json roto'

# COLISION DE NOMBRES DE PAR
$H1 = Crear 'ColisionA' @('x','x,y') @('a') 'x' @('x') @('x|a|x','x,y|a|x') 1
$H2 = Crear 'ColisionB' @('y,z','z') @('a') 'y,z' @('z') @('y,z|a|z','z|a|z') 1
Chk 'H1 colisionA valido' ($H1.body.valido -eq $true) "errores=$($H1.body.errores -join '; ')"
Chk 'H2 colisionB valido' ($H2.body.valido -eq $true) "errores=$($H2.body.errores -join '; ')"
$F12 = Post '/api/unir' "idA=$($H1.body.id)&idB=$($H2.body.id)"
Chk 'F12 union colisionable no revienta' (-not $F12.muerto) 'servidor cayo'
Chk 'F12 union invalida se bloquea' ($F12.body.exitoso -eq $false) "exitoso=$($F12.body.exitoso) -- se guardo un automata roto"
Chk 'F12 explica el motivo' ($F12.body.error -like '*comas*') "error=$($F12.body.error)"
Chk 'F12 detalla los errores del validador' ($F12.body.errores.Count -ge 1) "errores=$($F12.body.errores -join '; ')"

Chk 'F servidor sigue vivo' (Vivo) 'el servidor murio en la seccion F'

# =====================================================================
Seccion 'G. UNIONES: PARAMETROS INVALIDOS'
# =====================================================================

$tests = @(
  @{ n='id negativo';        b="idA=-1&idB=0" },
  @{ n='id no numerico';     b="idA=abc&idB=0" },
  @{ n='id fuera de rango';  b="idA=99999&idB=0" },
  @{ n='idB faltante';       b="idA=0" },
  @{ n='sin parametros';     b="" },
  @{ n='id vacio';           b="idA=&idB=0" },
  @{ n='id con espacio';     b="idA=%201&idB=0" },
  @{ n='id decimal';         b="idA=1.5&idB=0" },
  @{ n='id con signo mas';   b="idA=%2B1&idB=0" },
  @{ n='id hexadecimal';     b="idA=0x1&idB=0" }
)
foreach ($t in $tests) {
  $r = Post '/api/unir' $t.b
  Chk ("G union " + $t.n) (($r.status -eq 400) -and (-not $r.muerto)) "status=$($r.status) raw=$($r.raw)"
}

# desbordamiento de entero en el parseo de id
$r = Post '/api/unir' "idA=99999999999999999999&idB=0"
Chk 'G id gigante rechazado (posible desborde)' (($r.status -eq 400) -and (-not $r.muerto)) "status=$($r.status) body=$($r.raw)"

$r = Post '/api/unir' "idA=00000000000000000000000&idB=0"
Chk 'G id con muchos ceros no confunde' (-not $r.muerto) "status=$($r.status)"

Chk 'G servidor sigue vivo' (Vivo) 'el servidor murio en la seccion G'

# =====================================================================
Seccion 'H. PRUEBA DE CADENAS: MODO SIMPLE'
# =====================================================================

function ProbarSimple($id, $cadena) {
  return Post '/api/probar-cadena' ("modo=simple&id=$id&cadena=" + (Enc $cadena))
}

$casos = @(
  @{ c='a';    esperado=$true  },
  @{ c='b';    esperado=$false },
  @{ c='ba';   esperado=$true  },
  @{ c='ab';   esperado=$false },
  @{ c='abb';  esperado=$false },
  @{ c='abba'; esperado=$true  },
  @{ c='aaaa'; esperado=$true  },
  @{ c='bbbb'; esperado=$false },
  @{ c='';     esperado=$false }
)
foreach ($caso in $casos) {
  $r = ProbarSimple $idA1 $caso.c
  $et = $caso.c
  if ($et -eq '') { $et = '(vacia)' }
  Chk ("H TerminaEnA '" + $et + "'") ($r.body.recorridos[0].acepta -eq $caso.esperado) "obtuvo=$($r.body.recorridos[0].acepta) esperaba=$($caso.esperado) traza=$($r.body.recorridos[0].traza -join '>')"
}

# longitud de traza correcta
$r = ProbarSimple $idA1 'abba'
Chk 'H traza tiene largo+1' ($r.body.recorridos[0].traza.Count -eq 5) "traza=$($r.body.recorridos[0].traza.Count)"
Chk 'H recorridos = 1 en simple' ($r.body.recorridos.Count -eq 1) "recorridos=$($r.body.recorridos.Count)"
Chk 'H principal = 0' ($r.body.principal -eq 0) "principal=$($r.body.principal)"
Chk 'H modo simple' ($r.body.modo -eq 'simple') "modo=$($r.body.modo)"

# simbolo fuera del alfabeto
$r = ProbarSimple $idA1 'azb'
Chk 'H simbolo fuera de sigma: no completa' ($r.body.recorridos[0].completa -eq $false) "completa=$($r.body.recorridos[0].completa)"
Chk 'H reporta el simbolo trabado' ($r.body.recorridos[0].simboloTrabado -eq 'z') "trabado=$($r.body.recorridos[0].simboloTrabado)"
Chk 'H rechaza al trabarse' ($r.body.recorridos[0].acepta -eq $false) "acepta=$($r.body.recorridos[0].acepta)"
Chk 'H consumidos correcto' ($r.body.recorridos[0].consumidos -eq 1) "consumidos=$($r.body.recorridos[0].consumidos)"

# cadena larga
$larga = 'ab' * 500
$r = ProbarSimple $idA1 $larga
Chk 'H cadena de 1000 simbolos' (-not $r.muerto) 'servidor cayo'
Chk 'H traza de 1001 estados' ($r.body.recorridos[0].traza.Count -eq 1001) "traza=$($r.body.recorridos[0].traza.Count)"
Chk 'H termina en b => rechaza' ($r.body.recorridos[0].acepta -eq $false) "acepta=$($r.body.recorridos[0].acepta)"

# cadena con caracteres especiales
$r = ProbarSimple $idA1 'a&b=c%20d'
Chk 'H cadena con & = % no revienta' (-not $r.muerto) 'servidor cayo'
Chk 'H se traba en simbolo raro' ($r.body.recorridos[0].completa -eq $false) "completa=$($r.body.recorridos[0].completa)"

# simbolos url-encodeados como alfabeto real
$r = ProbarSimple $D6.body.id '&=&='
Chk 'H alfabeto & = funciona' ($r.body.recorridos[0].completa -eq $true) "completa=$($r.body.recorridos[0].completa) traza=$($r.body.recorridos[0].traza -join '>')"

$r = ProbarSimple $D7.body.id '+%+%'
Chk 'H alfabeto + % funciona' ($r.body.recorridos[0].completa -eq $true) "completa=$($r.body.recorridos[0].completa) trabado=$($r.body.recorridos[0].simboloTrabado)"

# mayusculas distintas de minusculas
$r = ProbarSimple $idD8 'aA'
Chk 'H mayus/minus distinguidas' ($r.body.recorridos[0].acepta -eq $true) "acepta=$($r.body.recorridos[0].acepta) traza=$($r.body.recorridos[0].traza -join '>')"
$r = ProbarSimple $idD8 'aa'
Chk 'H aa rechazada' ($r.body.recorridos[0].acepta -eq $false) "acepta=$($r.body.recorridos[0].acepta)"

# automata sin finales: nada se acepta
foreach ($c in @('','a','b','ab','ba')) {
  $r = ProbarSimple $idA4 $c
  $et = $c; if ($et -eq '') { $et = '(vacia)' }
  Chk ("H sin finales rechaza '" + $et + "'") ($r.body.recorridos[0].acepta -eq $false) "acepta=$($r.body.recorridos[0].acepta)"
}

# automata que acepta todo
foreach ($c in @('','a','b','abab')) {
  $r = ProbarSimple $idA3 $c
  $et = $c; if ($et -eq '') { $et = '(vacia)' }
  Chk ("H acepta-todo acepta '" + $et + "'") ($r.body.recorridos[0].acepta -eq $true) "acepta=$($r.body.recorridos[0].acepta)"
}

# un automata que no se guardo no se puede probar
$r = ProbarSimple $idA5 'a'
Chk 'H no se puede probar un automata invalido' ($r.status -eq 400) "status=$($r.status)"

# simbolo fuera del alfabeto: el recorrido se traba y lo dice
$r = ProbarSimple $idA3 'z'
Chk 'H simbolo fuera del alfabeto se traba' ($r.body.recorridos[0].completa -eq $false) "completa=$($r.body.recorridos[0].completa)"
Chk 'H informa el simbolo que traba' ($r.body.recorridos[0].simboloTrabado -eq 'z') "simboloTrabado=$($r.body.recorridos[0].simboloTrabado)"

Chk 'H servidor sigue vivo' (Vivo) 'el servidor murio en la seccion H'

# =====================================================================
Seccion 'I. PRUEBA DE CADENAS: MODO UNION Y VEREDICTO TRIPLE'
# =====================================================================

function ProbarUnion($u, $a, $b, $cadena) {
  return Post '/api/probar-cadena' ("modo=union&idUnion=$u&idA=$a&idB=$b&cadena=" + (Enc $cadena))
}

$casosU = @('','a','b','ab','ba','abb','abba','bb','bbaa','aabb')
foreach ($c in $casosU) {
  $r = ProbarUnion $idU1 $idA1 $idA2 $c
  $et = $c; if ($et -eq '') { $et = '(vacia)' }
  Chk ("I 3 recorridos '" + $et + "'") ($r.body.recorridos.Count -eq 3) "recorridos=$($r.body.recorridos.Count)"
  $ra = $r.body.recorridos[0].acepta
  $rb = $r.body.recorridos[1].acepta
  $ru = $r.body.recorridos[2].acepta
  $esperadoOR = ($ra -or $rb)
  Chk ("I union = A or B para '" + $et + "'") ($ru -eq $esperadoOR) "A=$ra B=$rb Union=$ru esperado=$esperadoOR"
}

$r = ProbarUnion $idU1 $idA1 $idA2 'abb'
Chk 'I principal = 2' ($r.body.principal -eq 2) "principal=$($r.body.principal)"
Chk 'I roles correctos' (($r.body.recorridos[0].rol -eq 'Automata 1') -and ($r.body.recorridos[2].rol -eq 'Automata union')) "roles=$($r.body.recorridos.rol -join ',')"
Chk 'I modo union' ($r.body.modo -eq 'union') "modo=$($r.body.modo)"

# traza del automata union con estados compuestos
Chk 'I traza union usa pares' ($r.body.recorridos[2].traza[0] -eq '(q0,p0)') "primer estado=$($r.body.recorridos[2].traza[0])"

# union anidada probada
$r = ProbarUnion $idU3 $idU1 $idA1 'abab'
Chk 'I union anidada probada' (-not $r.muerto) 'servidor cayo'
$ru = $r.body.recorridos[2].acepta
Chk 'I anidada = OR' ($ru -eq ($r.body.recorridos[0].acepta -or $r.body.recorridos[1].acepta)) "A=$($r.body.recorridos[0].acepta) B=$($r.body.recorridos[1].acepta) U=$ru"

# 16 estados anidados
$r = ProbarUnion $idU4 $idU3 $idA2 'abba'
Chk 'I union de 16 estados probada' (-not $r.muerto) 'servidor cayo'
Chk 'I traza correcta en anidada' ($r.body.recorridos[2].traza.Count -eq 5) "traza=$($r.body.recorridos[2].traza.Count)"

# mezclar automatas que no tienen nada que ver (permitido: son 3 pruebas independientes)
$r = ProbarUnion $idA1 $idA2 $idA3 'ab'
Chk 'I automatas no relacionados no revientan' (-not $r.muerto) 'servidor cayo'
Chk 'I sigue devolviendo 3' ($r.body.recorridos.Count -eq 3) "recorridos=$($r.body.recorridos.Count)"

Chk 'I servidor sigue vivo' (Vivo) 'el servidor murio en la seccion I'

# =====================================================================
Seccion 'J. PRUEBA DE CADENAS: PARAMETROS INVALIDOS'
# =====================================================================

$testsJ = @(
  @{ n='simple sin id';          b="modo=simple&cadena=a" },
  @{ n='simple id negativo';     b="modo=simple&id=-1&cadena=a" },
  @{ n='simple id fuera rango';  b="modo=simple&id=99999&cadena=a" },
  @{ n='simple id no numerico';  b="modo=simple&id=xyz&cadena=a" },
  @{ n='union sin idUnion';      b="modo=union&idA=0&idB=1&cadena=a" },
  @{ n='union sin idA';          b="modo=union&idUnion=0&idB=1&cadena=a" },
  @{ n='union ids invalidos';    b="modo=union&idUnion=x&idA=y&idB=z&cadena=a" },
  @{ n='union id fuera rango';   b="modo=union&idUnion=0&idA=1&idB=88888&cadena=a" },
  @{ n='sin modo ni ids';        b="cadena=a" },
  @{ n='cuerpo vacio';           b="" }
)
foreach ($t in $testsJ) {
  $r = Post '/api/probar-cadena' $t.b
  Chk ("J probar " + $t.n) (($r.status -eq 400) -and (-not $r.muerto)) "status=$($r.status) raw=$($r.raw)"
}

# modo desconocido cae a union y falta info -> 400
$r = Post '/api/probar-cadena' "modo=loquesea&id=0&cadena=a"
Chk 'J modo desconocido rechazado' (($r.status -eq 400) -and (-not $r.muerto)) "status=$($r.status)"

# sin cadena (deberia tratarse como vacia)
$r = Post '/api/probar-cadena' "modo=simple&id=$idA1"
Chk 'J sin parametro cadena = cadena vacia' (($r.status -eq 200) -and ($r.body.cadena -eq '')) "status=$($r.status) cadena=$($r.body.cadena)"

Chk 'J servidor sigue vivo' (Vivo) 'el servidor murio en la seccion J'

# =====================================================================
Seccion 'K. RUTAS Y METODOS'
# =====================================================================

$r = Get_ '/api/automata?id=0'
Chk 'K obtener automata por id' ($r.status -eq 200) "status=$($r.status)"
$r = Get_ '/api/automata?id=99999'
Chk 'K id inexistente 404' ($r.status -eq 404) "status=$($r.status)"
$r = Get_ '/api/automata'
Chk 'K sin id 404' ($r.status -eq 404) "status=$($r.status)"
$r = Get_ '/api/automata?id=-5'
Chk 'K id negativo 404' ($r.status -eq 404) "status=$($r.status)"
$r = Get_ '/api/noexiste'
Chk 'K ruta inexistente 404' ($r.status -eq 404) "status=$($r.status)"
$r = Get_ '/../CMakeLists.txt'
Chk 'K path traversal bloqueado' ($r.status -ne 200) "status=$($r.status) -- SE SIRVIO UN ARCHIVO FUERA DE static"
$r = Get_ '/..%2f..%2fCMakeLists.txt'
Chk 'K traversal codificado bloqueado' ($r.status -ne 200) "status=$($r.status)"
$r = Post '/api/validar' "id=$idA1"
Chk 'K validar guardado' (($r.status -eq 200) -and ($r.body.valido -eq $true)) "status=$($r.status) valido=$($r.body.valido)"
$r = Post '/api/validar' "id=99999"
Chk 'K validar id malo 400' ($r.status -eq 400) "status=$($r.status)"
$r = Get_ '/api/automatas'
Chk 'K listar sigue funcionando' ($r.status -eq 200) "status=$($r.status)"
Chk 'K lista no vacia' ($r.body.automatas.Count -gt 15) "cantidad=$($r.body.automatas.Count)"

# estatica
foreach ($f in @('/','/index.html','/styles.css','/app.js')) {
  $r = Get_ $f
  Chk ("K estatico " + $f) ($r.status -eq 200) "status=$($r.status)"
}

Chk 'K servidor sigue vivo' (Vivo) 'el servidor murio en la seccion K'

# =====================================================================
Seccion 'L. CARGA Y ESCALA'
# =====================================================================

# automata de 12 estados completo
$estados12 = @()
for ($i = 0; $i -lt 12; $i++) { $estados12 += "n$i" }
$trans12 = @()
for ($i = 0; $i -lt 12; $i++) {
  $sig = ($i + 1) % 12
  $trans12 += "n$i|a|n$sig"
  $trans12 += "n$i|b|n$i"
}
$L1 = Crear 'Doce' $estados12 @('a','b') 'n0' @('n11') $trans12 1
Chk 'L1 automata de 12 estados valido' ($L1.body.valido -eq $true) "errores=$($L1.body.errores -join '; ')"
$idL1 = $L1.body.id

$L2 = Post '/api/unir' "idA=$idL1&idB=$idA1"
Chk 'L2 union 12x2 = 24 estados' ($L2.body.automata.estados.Count -eq 24) "estados=$($L2.body.automata.estados.Count)"
Chk 'L2 48 transiciones' ($L2.body.automata.transiciones.Count -eq 48) "transiciones=$($L2.body.automata.transiciones.Count)"
Chk 'L2 construccion 24 pasos' ($L2.body.construccion.pasos.Count -eq 24) "pasos=$($L2.body.construccion.pasos.Count)"
$idL2 = $L2.body.id

$L3 = Post '/api/unir' "idA=$idL2&idB=$idL1"
Chk 'L3 union 24x12 = 288 estados' ($L3.body.automata.estados.Count -eq 288) "estados=$($L3.body.automata.estados.Count)"
Chk 'L3 no murio con 576 transiciones' (-not $L3.muerto) 'servidor cayo'
$idL3 = $L3.body.id

$r = ProbarSimple $idL3 ('ab' * 100)
Chk 'L4 cadena 200 sobre 288 estados' (-not $r.muerto) 'servidor cayo'
Chk 'L4 traza 201' ($r.body.recorridos[0].traza.Count -eq 201) "traza=$($r.body.recorridos[0].traza.Count)"

# muchas peticiones seguidas
$errores = 0
for ($i = 0; $i -lt 40; $i++) {
  $r = ProbarSimple $idA1 'abab'
  if ($r.muerto -or $r.status -ne 200) { $errores++ }
}
Chk 'L5 40 peticiones seguidas sin fallar' ($errores -eq 0) "fallidas=$errores"

# peticiones en paralelo
$trabajos = @()
for ($i = 0; $i -lt 8; $i++) {
  $trabajos += Start-Job -ScriptBlock {
    $ok = 0
    for ($k = 0; $k -lt 10; $k++) {
      try {
        $r = Invoke-WebRequest -Uri 'http://localhost:8080/api/automatas' -UseBasicParsing -TimeoutSec 20
        if ($r.StatusCode -eq 200) { $ok++ }
      } catch {}
    }
    return $ok
  }
}
$total = 0
foreach ($t in $trabajos) { $total += (Receive-Job -Job $t -Wait) }
Remove-Job -Job $trabajos -Force
Chk 'L6 80 peticiones concurrentes' ($total -eq 80) "exitosas=$total de 80"

Chk 'L servidor sigue vivo' (Vivo) 'el servidor murio en la seccion L'

# =====================================================================
Seccion 'M. CONSISTENCIA FINAL'
# =====================================================================

# utf-8 declarado en la respuesta
$r = Get_ '/api/automatas'
Chk 'M respuesta declara utf-8' ($r.raw.Length -gt 0) 'sin contenido'
$nUni = [char]0x00F1 + 'x'
$bUni = 'nombre=Acentos&estado=' + (Enc $nUni) + '&estado=q1&simbolo=a&inicial=' + (Enc $nUni)
$bUni += '&transicion=' + (Enc "$nUni|a|q1") + '&transicion=q1%7Ca%7Cq1&guardar=0'
$rUni = Post '/api/automatas' $bUni
Chk 'M nombre con enie vuelve intacto' ($rUni.body.automata.estados -contains $nUni) "estados=[$($rUni.body.automata.estados -join ' , ')] esperaba '$nUni'"

$r = Get_ '/api/automatas'
$lista = $r.body.automatas
Chk 'M ids consecutivos desde 0' ($lista[0].id -eq 0) "primer id=$($lista[0].id)"
$consecutivos = $true
for ($i = 0; $i -lt $lista.Count; $i++) {
  if ($lista[$i].id -ne $i) { $consecutivos = $false }
}
Chk 'M todos los ids coinciden con su posicion' $consecutivos 'los ids no son consecutivos'

# ningun automata guardado puede ser invalido
$invalidos = @()
foreach ($a in $lista) {
  $v = Post '/api/validar' "id=$($a.id)"
  if ($v.body.valido -ne $true) { $invalidos += "$($a.id):$($a.nombre)" }
}
Chk 'M todos los guardados son validos' ($invalidos.Count -eq 0) "invalidos=$($invalidos -join ', ')"

Write-Host ""
Write-Host "======================================================"
Write-Host "  automatas creados en el servidor: $($lista.Count)"
Write-Host "  pruebas pasadas : $script:pasadas"
Write-Host "  pruebas fallidas: $script:fallidas"
Write-Host "======================================================"
if ($script:fallidas -gt 0) {
  Write-Host ""
  Write-Host "DETALLE DE FALLOS:"
  foreach ($f in $script:fallos) { Write-Host "  - $f" }
}
