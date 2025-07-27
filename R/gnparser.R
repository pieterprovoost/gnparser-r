#' gnparser
#'
#' @docType package
#' @name gnparser
"_PACKAGE"

#' @useDynLib gnparser, .registration = TRUE
#' @import jsonlite dplyr
#' @importFrom Rcpp sourceCpp
#' @importFrom purrr map
NULL

#' Parse names using gnparser
#' 
#' @param json JSON string from the gnparser library
#' @return A data.frame
flatten_result <- function(json) {
    x <- fromJSON(json)
    data.frame(
        parsed = if (!is.null(x$parsed)) x$parsed else NA,
        quality = if (!is.null(x$quality)) x$quality else NA,
        verbatim = if (!is.null(x$verbatim)) x$verbatim else NA,
        normalized = if (!is.null(x$normalized)) x$normalized else NA,
        canonical_stemmed = if (!is.null(x$canonical$stemmed)) x$canonical$stemmed else NA,
        canonical_simple = if (!is.null(x$canonical$simple)) x$canonical$simple else NA,
        canonical_full = if (!is.null(x$canonical$full)) x$canonical$full else NA,
        authorship_verbatim = if (!is.null(x$authorship$verbatim)) x$authorship$verbatim else NA,
        authorship_normalized = if (!is.null(x$authorship$normalized)) x$authorship$normalized else NA,
        authorship_year = if (!is.null(x$authorship$year)) x$authorship$year else NA,
        cardinality = if (!is.null(x$cardinality)) x$cardinality else NA,
        rank = if (!is.null(x$rank)) x$rank else NA,
        stringsAsFactors = FALSE
    )
}

#' Parse names using gnparser
#'
#' @param names Character vector of names to parse
#' @param code Optional nomenclatural code (e.g., botanical, zoological) 
#' @param diaereses Preserve diaereses (default: TRUE)
#' @return A data.frame with parsed results (one row per input name)
#' @export
parse <- function(names, code = NULL, diaereses = TRUE) {
    parse_names_internal(names = names, format = "compact", code = code, details = FALSE, diaereses = diaereses) |>
        map(flatten_result) |>
        bind_rows()
}
